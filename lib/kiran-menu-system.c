/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 21:42:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 14:25:32
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-system.c
 */
#include "lib/kiran-menu-system.h"

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include "lib/helper.h"
#include "lib/kiran-menu-common.h"

struct _KiranMenuSystem
{
    KiranMenuUnit parent_instance;

    GSettings *settings;

    GHashTable *apps;

    GList *new_apps;
};

G_DEFINE_TYPE(KiranMenuSystem, kiran_menu_system, KIRAN_TYPE_MENU_UNIT)

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

KiranMenuApp *kiran_menu_system_lookup_apps_with_window(KiranMenuSystem *self,
                                                        WnckWindow *window)
{
    RETURN_VAL_IF_FALSE(window != NULL, NULL);

    const gchar *instance_name = wnck_window_get_class_instance_name(window);
    const gchar *group_name = wnck_window_get_class_group_name(window);

    typedef enum
    {
        MATCH_NONE,
        MATCH_ALL_ONCE,
        MATCH_ALL_MULTIPLE,
        MATCH_PART_ONCE,
        MATCH_PART_MULTIPLE,
    } WindowMatchType;

    KiranMenuApp *match_menu_app = NULL;
    GHashTableIter iter;
    gpointer key = NULL;
    KiranMenuApp *app = NULL;
    WindowMatchType match_type = MATCH_NONE;
    g_hash_table_iter_init(&iter, self->apps);
    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&app))
    {
        const gchar *exec = kiran_app_get_exec(KIRAN_APP(app));
        const gchar *locale_name = kiran_app_get_locale_name(KIRAN_APP(app));
        g_autofree gchar *exec_base_name = NULL;

        if (exec)
        {
            g_auto(GStrv) exec_split = g_strsplit(exec, " ", -1);
            if (exec_split && exec_split[0])
            {
                exec_base_name = g_path_get_basename(exec_split[0]);
            }
        }

        gboolean match_instance_name = (g_strcmp0(exec_base_name, instance_name) == 0);
        gboolean match_group_name = (g_strcmp0(group_name, locale_name) == 0);

        if (match_instance_name && match_group_name)
        {
            if (match_type == MATCH_ALL_ONCE || match_type == MATCH_ALL_MULTIPLE)
            {
                match_type = MATCH_ALL_MULTIPLE;
            }
            else
            {
                match_menu_app = g_object_ref(app);
                match_type = MATCH_ALL_ONCE;
            }
        }
        else if ((match_instance_name || match_group_name) &&
                 match_type != MATCH_ALL_ONCE &&
                 match_type != MATCH_ALL_MULTIPLE)
        {
            if (match_type == MATCH_PART_ONCE || match_type == MATCH_PART_MULTIPLE)
            {
                match_type = MATCH_PART_MULTIPLE;
            }
            else
            {
                match_menu_app = g_object_ref(app);
                match_type = MATCH_PART_ONCE;
            }
        }
    }
    if (match_type == MATCH_ALL_MULTIPLE)
    {
        g_warning("Multiple App match the window in terms of instance name and group name.");
    }
    else if (match_type == MATCH_PART_MULTIPLE)
    {
        g_warning("Multiple App match the window in terms of instance name or group name.");
    }

    return match_menu_app;
}

GList *kiran_menu_system_get_nnew_apps(KiranMenuSystem *self, gint top_n)
{
    GList *new_apps = NULL;

    for (GList *l = self->new_apps; l != NULL; l = l->next)
    {
        GQuark quark = GPOINTER_TO_UINT(l->data);

        const char *desktop_id = g_quark_to_string(quark);
        new_apps = g_list_append(new_apps, g_strdup(desktop_id));
    }

    if (top_n < 0)
    {
        return new_apps;
    }
    else
    {
        return list_remain_headn(new_apps, top_n, g_free);
    }
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

static void kiran_menu_system_flush(KiranMenuUnit *unit, gpointer user_data)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(unit);
    GList *registered_apps = g_app_info_get_all();

    gboolean new_app_change = FALSE;

    // new installed apps
    if (self->apps)
    {
        for (GList *l = registered_apps; l != NULL; l = l->next)
        {
            GAppInfo *app_info = l->data;
            if (g_app_info_should_show(app_info))
            {
                const gchar *desktop_id = g_app_info_get_id(app_info);
                GQuark quark = g_quark_from_string(desktop_id);
                if (g_hash_table_lookup(self->apps, GUINT_TO_POINTER(quark)) == NULL &&
                    g_list_find(self->new_apps, GUINT_TO_POINTER(quark)) == NULL)
                {
                    self->new_apps = g_list_append(self->new_apps, GUINT_TO_POINTER(quark));
                    new_app_change = TRUE;
                }
            }
        }
    }

    // update system apps
    g_clear_pointer(&self->apps, (GDestroyNotify)g_hash_table_unref);
    self->apps = g_hash_table_new_full(NULL, NULL, NULL, g_object_unref);

    for (GList *l = registered_apps; l != NULL; l = l->next)
    {
        GAppInfo *app_info = l->data;
        if (g_app_info_should_show(app_info))
        {
            const gchar *desktop_id = g_app_info_get_id(app_info);
            GQuark quark = g_quark_from_string(desktop_id);
            g_hash_table_insert(self->apps, GUINT_TO_POINTER(quark),
                                kiran_menu_app_get_new(desktop_id));
        }
    }
    g_list_free_full(registered_apps, g_object_unref);

    // uninstalled apps
    for (GList *l = self->new_apps; l != NULL;)
    {
        if (g_hash_table_lookup(self->apps, l->data) == NULL)
        {
            GList *t = l;
            l = l->next;
            self->new_apps = g_list_delete_link(self->new_apps, t);
            new_app_change = TRUE;
        }
        else
        {
            l = l->next;
        }
    }

    if (new_app_change)
    {
        write_list_quark_to_as(self->settings, "new-apps", self->new_apps);
    }
}

static void read_new_apps(KiranMenuSystem *self)
{
    g_clear_pointer(&self->new_apps, g_list_free);
    self->new_apps = read_as_to_list_quark(self->settings, "new-apps");
}

static void monitor_window_open(WnckScreen *screen,
                                WnckWindow *window,
                                gpointer user_data)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(user_data);

    KiranMenuApp *menu_app = kiran_menu_system_lookup_apps_with_window(self, window);
    const gchar *desktop_id = kiran_app_get_desktop_id(KIRAN_APP(menu_app));

    GQuark quark = g_quark_from_string(desktop_id);
    if (g_list_find(self->new_apps, GUINT_TO_POINTER(quark)))
    {
        self->new_apps = g_list_remove(self->new_apps, GUINT_TO_POINTER(quark));
        write_list_quark_to_as(self->settings, "new-apps", self->new_apps);
    }
}

static void kiran_menu_system_init(KiranMenuSystem *self)
{
    self->settings = g_settings_new(KIRAN_MENU_SCHEMA);
    self->apps = NULL;
    self->new_apps = NULL;

    kiran_menu_system_flush(KIRAN_MENU_UNIT(self), NULL);

    read_new_apps(self);

    WnckScreen *screen = wnck_screen_get_default();
    if (screen)
    {
        wnck_screen_force_update(screen);
        g_signal_connect(screen, "window-opened", G_CALLBACK(monitor_window_open), self);

        // GList *windows = wnck_screen_get_windows(screen);
        // for (GList *l = windows; l != NULL; l = l->next)
        // {
        //     WnckWindow *window = l->data;
        //     GList *match_apps = kiran_menu_system_lookup_apps_with_window(self, window);

        //     g_print("match %u: %s %s\n",
        //             (match_apps != NULL),
        //             wnck_window_get_class_instance_name(window),
        //             wnck_window_get_class_group_name(window));
        // }
    }
    else
    {
        g_warning("the default screen is NULL. please run in GUI application.");
    }
}

static void kiran_menu_system_dispose(GObject *object)
{
    KiranMenuSystem *self = KIRAN_MENU_SYSTEM(object);

    g_clear_pointer(&self->apps, (GDestroyNotify)g_hash_table_unref);
    g_clear_pointer(&self->new_apps, g_list_free);

    G_OBJECT_CLASS(kiran_menu_system_parent_class)->dispose(object);
}

static void kiran_menu_system_class_init(KiranMenuSystemClass *klass)
{
    KiranMenuUnitClass *unit_class = KIRAN_MENU_UNIT_CLASS(klass);
    unit_class->flush = kiran_menu_system_flush;

    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_system_dispose;
}

KiranMenuSystem *kiran_menu_system_get_new()
{
    return g_object_new(KIRAN_TYPE_MENU_SYSTEM, NULL);
}
