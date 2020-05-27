#include "lib/kiran-menu-favorite.h"

#include "lib/kiran-app.h"
#include "lib/kiran-menu-common.h"

struct _KiranMenuFavorite
{
    KiranMenuUnit parent_instance;
    GSettings *settings;
    GList *favorite_apps;
};

G_DEFINE_TYPE(KiranMenuFavorite, kiran_menu_favorite, KIRAN_TYPE_MENU_UNIT)

enum
{
    SIGNAL_APP_ADDED,
    SIGNAL_APP_DELETED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

gboolean kiran_menu_favorite_add_app(KiranMenuFavorite *self,
                                     const char *desktop_id)
{
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);

    if (g_list_find(self->favorite_apps, GUINT_TO_POINTER(quark)) == NULL)
    {
        self->favorite_apps = g_list_append(self->favorite_apps, GUINT_TO_POINTER(quark));

        GList *add_apps = NULL;
        add_apps = g_list_append(add_apps, g_strdup(desktop_id));
        g_signal_emit(self, signals[SIGNAL_APP_ADDED], 0, add_apps);
        g_list_free_full(add_apps, g_free);

        return write_list_quark_to_as(self->settings, MENU_KEY_FAVORITE_APPS, self->favorite_apps);
    }
    return FALSE;
}

gboolean kiran_menu_favorite_del_app(KiranMenuFavorite *self,
                                     const char *desktop_id)
{
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);

    if (g_list_find(self->favorite_apps, GUINT_TO_POINTER(quark)) != NULL)
    {
        self->favorite_apps = g_list_remove_all(self->favorite_apps, GUINT_TO_POINTER(quark));

        GList *delete_apps = NULL;
        delete_apps = g_list_append(delete_apps, g_strdup(desktop_id));
        g_signal_emit(self, signals[SIGNAL_APP_DELETED], 0, delete_apps);
        g_list_free_full(delete_apps, g_free);

        return write_list_quark_to_as(self->settings, MENU_KEY_FAVORITE_APPS, self->favorite_apps);
    }
    return FALSE;
}

gboolean kiran_menu_favorite_find_app(KiranMenuFavorite *self,
                                      const char *desktop_id)
{
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);

    return (g_list_find(self->favorite_apps, GUINT_TO_POINTER(quark)) != NULL);
}

void kiran_menu_favorite_flush(KiranMenuUnit *unit, gpointer user_data)
{
    KiranMenuFavorite *self = KIRAN_MENU_FAVORITE(unit);
    GList *apps = (GList *)user_data;
    g_autoptr(GHashTable) app_table = g_hash_table_new(NULL, NULL);

    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        const gchar *desktop_id = kiran_app_get_desktop_id(app);
        GQuark quark = g_quark_from_string(desktop_id);
        g_hash_table_insert(app_table, GUINT_TO_POINTER(quark), GUINT_TO_POINTER(TRUE));
    }

    GList *delete_apps = NULL;
    GList *iter = self->favorite_apps;
    while (iter)
    {
        if (g_hash_table_lookup(app_table, iter->data) == NULL)
        {
            GQuark quark = GPOINTER_TO_UINT(iter->data);
            const gchar *desktop_id = g_quark_to_string(quark);
            delete_apps = g_list_append(delete_apps, g_strdup(desktop_id));

            GList *tmp = iter;
            iter = iter->next;
            self->favorite_apps = g_list_delete_link(self->favorite_apps, tmp);
        }
        else
        {
            iter = iter->next;
        }
    }

    if (delete_apps != NULL)
    {
        write_list_quark_to_as(self->settings, MENU_KEY_FAVORITE_APPS, self->favorite_apps);
        g_signal_emit(self, signals[SIGNAL_APP_DELETED], 0, delete_apps);
        g_list_free_full(delete_apps, g_free);
    }
}

GList *kiran_menu_favorite_get_favorite_apps(KiranMenuFavorite *self)
{
    GList *apps = NULL;
    for (GList *l = self->favorite_apps; l != NULL; l = l->next)
    {
        GQuark quark = GPOINTER_TO_UINT(l->data);
        const gchar *desktop_id = g_quark_to_string(quark);
        apps = g_list_append(apps, g_strdup(desktop_id));
    }
    return apps;
}

static void app_changed(GSettings *settings,
                        gchar *key,
                        gpointer user_data)
{

    if (g_strcmp0(key, MENU_KEY_FAVORITE_APPS) != 0)
    {
        return;
    }

    KiranMenuFavorite *self = KIRAN_MENU_FAVORITE(user_data);

    GList *new_favorite_apps = read_as_to_list_quark(self->settings, MENU_KEY_FAVORITE_APPS);

    GList *add_apps = NULL;
    GList *delete_apps = NULL;

    for (GList *l = self->favorite_apps; l != NULL; l = l->next)
    {
        if (g_list_find(new_favorite_apps, l->data) == NULL)
        {
            GQuark quark = GPOINTER_TO_UINT(l->data);
            const gchar *desktop_id = g_quark_to_string(quark);
            delete_apps = g_list_append(delete_apps, g_strdup(desktop_id));
        }
    }

    for (GList *l = new_favorite_apps; l != NULL; l = l->next)
    {
        if (g_list_find(self->favorite_apps, l->data) == NULL)
        {
            GQuark quark = GPOINTER_TO_UINT(l->data);
            const gchar *desktop_id = g_quark_to_string(quark);
            add_apps = g_list_append(add_apps, g_strdup(desktop_id));
        }
    }

    g_list_free(self->favorite_apps);
    self->favorite_apps = new_favorite_apps;

    if (delete_apps)
    {
        g_signal_emit(self, signals[SIGNAL_APP_DELETED], 0, delete_apps);
        g_list_free_full(delete_apps, g_free);
    }

    if (add_apps)
    {
        g_signal_emit(self, signals[SIGNAL_APP_ADDED], 0, add_apps);
        g_list_free_full(add_apps, g_free);
    }
}

static void kiran_menu_favorite_init(KiranMenuFavorite *self)
{
    self->settings = g_settings_new(KIRAN_MENU_SCHEMA);

    self->favorite_apps = read_as_to_list_quark(self->settings, MENU_KEY_FAVORITE_APPS);

    g_signal_connect(self->settings, "changed::" MENU_KEY_FAVORITE_APPS, G_CALLBACK(app_changed), self);
}

static void kiran_menu_favorite_dispose(GObject *object)
{
    KiranMenuFavorite *menu_favorite = KIRAN_MENU_FAVORITE(object);

    g_clear_pointer(&menu_favorite->settings, g_object_unref);
    g_clear_pointer(&menu_favorite->favorite_apps, g_list_free);

    G_OBJECT_CLASS(kiran_menu_favorite_parent_class)->dispose(object);
}

static void kiran_menu_favorite_class_init(KiranMenuFavoriteClass *klass)
{
    KiranMenuUnitClass *unit_class = KIRAN_MENU_UNIT_CLASS(klass);
    unit_class->flush = kiran_menu_favorite_flush;

    signals[SIGNAL_APP_ADDED] = g_signal_new("app-added",
                                             KIRAN_TYPE_MENU_FAVORITE,
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL,
                                             NULL,
                                             NULL,
                                             G_TYPE_NONE,
                                             1,
                                             G_TYPE_POINTER);

    signals[SIGNAL_APP_DELETED] = g_signal_new("app-deleted",
                                               KIRAN_TYPE_MENU_FAVORITE,
                                               G_SIGNAL_RUN_LAST,
                                               0,
                                               NULL,
                                               NULL,
                                               NULL,
                                               G_TYPE_NONE,
                                               1,
                                               G_TYPE_POINTER);

    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_favorite_dispose;
}

KiranMenuFavorite *kiran_menu_favorite_get_new(void)
{
    return g_object_new(KIRAN_TYPE_MENU_FAVORITE, NULL);
}
