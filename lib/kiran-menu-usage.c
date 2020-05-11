/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 20:35:20
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 17:59:39
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-usage.c
 */
#include "lib/kiran-menu-usage.h"

#include <libwnck/libwnck.h>

#include "lib/helper.h"
#include "lib/kiran-menu-common.h"
#include "lib/kiran-menu-skeleton.h"
#include "lib/kiran-menu-system.h"
#include "lib/math_helper.h"

struct _KiranMenuUsage
{
    KiranMenuUnit parent_instance;

    GSettings *settings;

    GHashTable *app_usages;

    long watch_start_time;
    gchar *focus_desktop_id;

    guint save_id;

    GDBusProxy *session_proxy;
    gboolean screen_idle;
};

G_DEFINE_TYPE(KiranMenuUsage, kiran_menu_usage, KIRAN_TYPE_MENU_UNIT);

enum
{
    SIGNAL_APP_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

#define FOCUS_TIME_MIN_SECONDS 2
#define FOCUS_TIME_MAX_SECONDS 1000
#define EPS 1e-8

/* If we transition to idle, only count this many seconds of usage */
#define IDLE_TIME_TRANSITION_SECONDS 30

/* http://www.gnome.org/~mccann/gnome-session/docs/gnome-session.html#org.gnome.SessionManager.Presence */
#define GNOME_SESSION_STATUS_IDLE 3

#define SAVE_APPS_TIMEOUT_SECONDS 10

typedef struct UsageData UsageData;
struct UsageData
{
    gdouble score;
    long last_seen;
};

static long get_system_time(void)
{
    return g_get_real_time() / G_TIME_SPAN_SECOND;
}

static UsageData *get_usage_for_app(KiranMenuUsage *self,
                                    const char *desktop_id)
{
    UsageData *usage;

    GQuark quark = g_quark_from_string(desktop_id);

    usage = g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(quark));
    if (usage) return usage;

    usage = g_new0(UsageData, 1);
    g_hash_table_insert(self->app_usages, GUINT_TO_POINTER(quark), usage);
    return usage;
}

static gboolean write_usages_to_settings(gpointer data)
{
    KiranMenuUsage *self = KIRAN_MENU_USAGE(data);

    self->save_id = 0;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    GHashTableIter iter;
    gpointer key;
    UsageData *usage;
    g_hash_table_iter_init(&iter, self->app_usages);
    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&usage))
    {
        GVariant *v_usage = g_variant_new("(di)", usage->score, usage->last_seen);
        GQuark quark = GPOINTER_TO_UINT(key);
        const char *desktop_id = g_quark_to_string(quark);
        g_variant_builder_add(&builder, "{sv}", desktop_id,
                              g_variant_new_variant(v_usage));
    }
    GVariant *frequent_apps = g_variant_builder_end(&builder);
    g_settings_set_value(self->settings, "frequent-apps", frequent_apps);
    return FALSE;
}

static gboolean read_usages_from_settings(KiranMenuUsage *self)
{
    g_autoptr(GVariant) frequent_apps =
        g_settings_get_value(self->settings, "frequent-apps");

    if (frequent_apps)
    {
        gsize child_num = g_variant_n_children(frequent_apps);
        for (gsize i = 0; i < child_num; ++i)
        {
            gchar *desktop_id;
            GVariant *v_usage;
            UsageData *usage = g_new0(UsageData, 1);

            g_variant_get_child(frequent_apps, i, "{sv}", &desktop_id, &v_usage);
            v_usage = g_variant_get_variant(v_usage);
            g_variant_get(v_usage, "(di)", &(usage->score), &(usage->last_seen));

            GQuark quark = g_quark_from_string(desktop_id);
            if (!g_hash_table_insert(self->app_usages, GUINT_TO_POINTER(quark),
                                     usage))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

static void ensure_queued_save(KiranMenuUsage *self)
{
    if (self->save_id != 0)
        return;
    self->save_id = g_timeout_add_seconds(SAVE_APPS_TIMEOUT_SECONDS, write_usages_to_settings, self);
    g_source_set_name_by_id(self->save_id, "[kiran-menu] save usage data in idle.");
}

static void increment_usage_for_app_at_time(KiranMenuUsage *self,
                                            char *desktop_id,
                                            long time)
{
    UsageData *usage;
    usage = get_usage_for_app(self, desktop_id);

    usage->last_seen = time;

    guint elapsed = time - self->watch_start_time;

    if (elapsed >= FOCUS_TIME_MIN_SECONDS)
    {
        elapsed = (elapsed > FOCUS_TIME_MAX_SECONDS) ? FOCUS_TIME_MAX_SECONDS : elapsed;
        usage->score += kiran_math_log10(elapsed);
    }

    ensure_queued_save(self);

    g_signal_emit(self, signals[SIGNAL_APP_CHANGED], 0);
}

static int sort_apps_by_usage(gconstpointer a, gconstpointer b, gpointer data)
{
    GQuark qa = g_quark_from_string((const gchar *)a);
    GQuark qb = g_quark_from_string((const gchar *)b);
    KiranMenuUsage *self = KIRAN_MENU_USAGE(data);

    UsageData *usage_a =
        (UsageData *)g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(qa));
    UsageData *usage_b =
        (UsageData *)g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(qb));

    gdouble a_score = (usage_a == NULL) ? 0 : usage_a->score;
    gdouble b_score = (usage_b == NULL) ? 0 : usage_b->score;
    if (ABS(a_score - b_score) < EPS) return 0;
    return a_score > b_score ? -1 : 1;
}

GList *kiran_menu_usage_get_nfrequent_apps(KiranMenuUsage *self, gint top_n)
{
    GHashTableIter iter;
    gpointer key;
    UsageData *value;
    GList *apps = NULL;

    g_hash_table_iter_init(&iter, self->app_usages);
    while (g_hash_table_iter_next(&iter, (gpointer)&key, (gpointer)&value))
    {
        if (value->score > EPS)
        {
            GQuark quark = GPOINTER_TO_UINT(key);
            const char *desktop_id = g_quark_to_string(quark);
            apps = g_list_append(apps, g_strdup(desktop_id));
        }
    }
    apps = g_list_sort_with_data(apps, sort_apps_by_usage, self);
    if (top_n < 0)
    {
        return apps;
    }
    else
    {
        return list_remain_headn(apps, top_n, g_free);
    }
}

gboolean kiran_menu_usage_reset(KiranMenuUsage *self)
{
    g_clear_pointer(&self->app_usages, g_hash_table_unref);
    g_clear_pointer(&self->focus_desktop_id, g_free);
    write_usages_to_settings(self);
    return TRUE;
}

void active_window_changed(WnckScreen *screen,
                           WnckWindow *previously_active_window,
                           gpointer user_data)
{
    KiranMenuUsage *self = KIRAN_MENU_USAGE(user_data);
    UsageData *usage_data = NULL;
    long cur_system_time = get_system_time();

    if (self->focus_desktop_id != NULL)
    {
        increment_usage_for_app_at_time(self, self->focus_desktop_id, cur_system_time);
        g_clear_pointer(&self->focus_desktop_id, g_free);
    }

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(kiran_menu_based_skeleton_get());
    g_return_if_fail(skeleton != NULL);

    KiranMenuSystem *menu_system = (KiranMenuSystem *)kiran_menu_skeleton_get_unit(skeleton, KIRAN_MENU_TYPE_SYSTEM);
    g_return_if_fail(menu_system != NULL);

    WnckWindow *active_window = wnck_screen_get_active_window(screen);
    KiranMenuApp *menu_app = kiran_menu_system_lookup_apps_with_window(menu_system, active_window);

    if (menu_app)
    {
        const gchar *desktop_id = kiran_app_get_desktop_id(KIRAN_APP(menu_app));
        self->focus_desktop_id = g_strdup(desktop_id);
        usage_data = get_usage_for_app(self, self->focus_desktop_id);
        usage_data->last_seen = cur_system_time;
    }
    self->watch_start_time = cur_system_time;
}

static void on_session_status_changed(GDBusProxy *proxy,
                                      guint status,
                                      KiranMenuUsage *self)
{
    gboolean idle;

    idle = (status >= GNOME_SESSION_STATUS_IDLE);
    if (self->screen_idle == idle)
        return;

    self->screen_idle = idle;
    if (idle)
    {
        long end_time;

        if (self->focus_desktop_id)
        {
            end_time = self->watch_start_time + IDLE_TIME_TRANSITION_SECONDS;
            increment_usage_for_app_at_time(self, self->focus_desktop_id, end_time);
        }
    }
    else
    {
        self->watch_start_time = get_system_time();
    }
}

static void session_proxy_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
    if (g_str_equal(signal_name, "StatusChanged"))
    {
        guint status;
        g_variant_get(parameters, "(u)", &status);
        on_session_status_changed(proxy, status, KIRAN_MENU_USAGE(user_data));
    }
}

static void kiran_menu_usage_init(KiranMenuUsage *self)
{
    self->settings = g_settings_new(KIRAN_MENU_SCHEMA);
    self->app_usages = g_hash_table_new_full(NULL, NULL, NULL, g_free);

    self->save_id = 0;

    self->screen_idle = FALSE;

    read_usages_from_settings(self);

    WnckScreen *screen = wnck_screen_get_default();
    if (screen)
    {
        wnck_screen_force_update(screen);
        g_signal_connect(screen, "active-window-changed", G_CALLBACK(active_window_changed), self);
    }
    else
    {
        g_warning("the default screen is NULL. please run in GUI application.");
    }

    GDBusConnection *session_bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    self->session_proxy = g_dbus_proxy_new_sync(session_bus,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL, /* interface info */
                                                "org.gnome.SessionManager",
                                                "/org/gnome/SessionManager/Presence",
                                                "org.gnome.SessionManager",
                                                NULL, /* cancellable */
                                                NULL /* error */);
    g_signal_connect(self->session_proxy, "g-signal", G_CALLBACK(session_proxy_signal), self);
    g_object_unref(session_bus);
}

static void kiran_menu_usage_dispose(GObject *object)
{
    KiranMenuUsage *self = KIRAN_MENU_USAGE(object);

    g_clear_pointer(&self->settings, g_object_unref);

    g_clear_pointer(&self->app_usages, g_hash_table_unref);
    g_clear_pointer(&self->focus_desktop_id, g_free);

    g_clear_pointer(&self->session_proxy, g_object_unref);

    if (self->save_id > 0)
        g_source_remove(self->save_id);

    G_OBJECT_CLASS(kiran_menu_usage_parent_class)->dispose(object);
}

static void kiran_menu_usage_class_init(KiranMenuUsageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    signals[SIGNAL_APP_CHANGED] = g_signal_new("app-changed",
                                               KIRAN_TYPE_MENU_USAGE,
                                               G_SIGNAL_RUN_LAST,
                                               0,
                                               NULL,
                                               NULL,
                                               NULL,
                                               G_TYPE_NONE,
                                               0);

    gobject_class->dispose = kiran_menu_usage_dispose;
}

/**
 * kiran_menu_usage_get_default:
 *
 * Return Value: (transfer none): The global #KiranMenuUsage instance
 */
KiranMenuUsage *kiran_menu_usage_get_new()
{
    return g_object_new(KIRAN_TYPE_MENU_USAGE, NULL);
}
