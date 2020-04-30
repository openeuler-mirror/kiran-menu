/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 21:42:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:58:29
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-system.c
 */
#include "src/start-menu/kiran-menu-system.h"

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include "src/utils/helper.h"

struct _KiranMenuSystem
{
    GObject parent;
    GHashTable *apps;
};

G_DEFINE_TYPE(KiranMenuSystem, kiran_menu_system, G_TYPE_OBJECT)

enum
{
    INSTALLED_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

GList *kiran_menu_system_get_apps(KiranMenuSystem *self)
{
    GList *apps = NULL;
    GHashTableIter iter;
    gpointer key = NULL;
    KiranMenuApp *app = NULL;
    g_hash_table_iter_init(&iter, self->apps);
    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&app))
    {
        apps = g_list_append(apps, g_object_ref(app));
    }
    return apps;
}

KiranMenuApp *kiran_menu_system_lookup_app(KiranMenuSystem *self,
                                           const gchar *desktop_id)
{
    GQuark quark = g_quark_from_string(desktop_id);
    return g_hash_table_lookup(self->apps, GUINT_TO_POINTER(quark));
}

static gint sort_by_ctime(gconstpointer a, gconstpointer b,
                          gpointer user_data)
{
    GQuark qa = g_quark_from_string((const gchar *)a);
    GQuark qb = g_quark_from_string((const gchar *)b);
    GHashTable *apps_ctime = user_data;

    gpointer timea = g_hash_table_lookup(apps_ctime, GUINT_TO_POINTER(qa));
    gpointer timeb = g_hash_table_lookup(apps_ctime, GUINT_TO_POINTER(qb));
    return GPOINTER_TO_UINT(timeb) - GPOINTER_TO_UINT(timea);
}

GList *kiran_menu_system_get_nnew_apps(KiranMenuSystem *self, gint top_n)
{
    GList *new_apps = NULL;
    g_autoptr(GHashTable) apps_ctime = g_hash_table_new(NULL, NULL);

    GHashTableIter iter;
    gpointer *key = NULL;
    KiranMenuApp *menu_app = NULL;

    g_hash_table_iter_init(&iter, self->apps);
    while (
        g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&menu_app))
    {
        GQuark quark = GPOINTER_TO_UINT(key);
        const char *desktop_id = g_quark_to_string(quark);
        new_apps = g_list_append(new_apps, g_strdup(desktop_id));

        guint64 ctime = kiran_app_get_create_time(KIRAN_APP(menu_app));
        g_hash_table_insert(apps_ctime, key, GUINT_TO_POINTER((ctime >> 32)));
    }

    new_apps = g_list_sort_with_data(new_apps, sort_by_ctime, apps_ctime);
    return list_remain_headn(new_apps, top_n, g_free);
}

gint sort_by_app_name(gconstpointer a, gconstpointer b, gpointer user_data)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(user_data);

    KiranMenuApp *appa = kiran_menu_system_lookup_app(self, (const gchar *)a);
    KiranMenuApp *appb = kiran_menu_system_lookup_app(self, (const gchar *)b);

    const char *appa_name = kiran_app_get_name(KIRAN_APP(appa));
    const char *appb_name = kiran_app_get_name(KIRAN_APP(appb));

    return g_strcmp0(appa_name, appb_name);
}

GList *kiran_menu_system_get_all_sorted_apps(KiranMenuSystem *self)
{
    GList *apps = NULL;
    GHashTableIter iter;
    gpointer key;
    KiranMenuApp *app;

    g_hash_table_iter_init(&iter, self->apps);
    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&app))
    {
        const char *desktop_id = kiran_app_get_desktop_id(KIRAN_APP(app));
        apps = g_list_append(apps, g_strdup(desktop_id));
    }

    return g_list_sort_with_data(apps, sort_by_app_name, self);
}

static void installed_app_change(GAppInfoMonitor *gappinfomonitor,
                                 gpointer user_data)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(user_data);
    g_clear_pointer(&self->apps, (GDestroyNotify)g_hash_table_unref);

    self->apps = g_hash_table_new_full(NULL, NULL, NULL, g_object_unref);
    GList *registered_apps = g_app_info_get_all();
    for (GList *l = registered_apps; l != NULL; l = l->next)
    {
        GAppInfo *info = l->data;
        if (g_app_info_should_show(info))
        {
            const gchar *desktop_id = g_app_info_get_id(info);
            GQuark quark = g_quark_from_string(desktop_id);
            g_hash_table_insert(self->apps, GUINT_TO_POINTER(quark),
                                kiran_menu_app_get_new(desktop_id));
        }
    }
    g_list_free_full(registered_apps, g_object_unref);
    g_signal_emit(self, signals[INSTALLED_CHANGED], 0, NULL);
}

static void kiran_menu_system_init(KiranMenuSystem *self)
{
    GAppInfoMonitor *monitor = g_app_info_monitor_get();
    g_signal_connect(monitor, "changed", G_CALLBACK(installed_app_change), self);
    installed_app_change(monitor, self);
}

static void kiran_menu_system_dispose(GObject *object)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(object);
    g_clear_pointer(&self->apps, (GDestroyNotify)g_hash_table_unref);
    G_OBJECT_CLASS(kiran_menu_system_parent_class)->dispose(object);
}

static void kiran_menu_system_class_init(KiranMenuSystemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_system_dispose;

    signals[INSTALLED_CHANGED] =
        g_signal_new("installed-changed", KIRAN_TYPE_MENU_SYSTEM,
                     G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

KiranMenuSystem *kiran_menu_system_get_new()
{
    return g_object_new(KIRAN_TYPE_MENU_SYSTEM, NULL);
}
