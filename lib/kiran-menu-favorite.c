#include "lib/kiran-menu-favorite.h"

#include "lib/kiran-app.h"
#include "lib/kiran-menu-common.h"

struct _KiranMenuFavorite
{
    GObject parent;
    GSettings *settings;
    GHashTable *favorite_apps;
};

G_DEFINE_TYPE(KiranMenuFavorite, kiran_menu_favorite, G_TYPE_OBJECT)

static gboolean write_favorite_to_settings(KiranMenuFavorite *self)
{
    GArray *apps = g_array_new(FALSE, FALSE, sizeof(gchar *));
    GHashTableIter iter;
    gchar *desktop_id = NULL;
    guint *value = NULL;
    g_hash_table_iter_init(&iter, self->favorite_apps);
    while (g_hash_table_iter_next(&iter, (gpointer *)&desktop_id,
                                  (gpointer *)&value))
    {
        gchar *dup_id = g_strdup(desktop_id);
        g_array_append_val(apps, dup_id);
    }

    char *null = NULL;
    g_array_append_val(apps, null);

    g_auto(GStrv) favorite_apps = (gchar **)g_array_free(apps, FALSE);
    return g_settings_set_strv(self->settings, "favorite-apps",
                               (const gchar *const *)favorite_apps);
}

static gboolean read_favorite_from_settings(KiranMenuFavorite *self)
{
    g_clear_pointer(&self->favorite_apps, g_hash_table_unref);
    self->favorite_apps =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    g_auto(GStrv) favorite_apps =
        g_settings_get_strv(self->settings, "favorite-apps");

    for (gint i = 0; favorite_apps[i] != NULL; ++i)
    {
        if (!g_hash_table_insert(self->favorite_apps, g_strdup(favorite_apps[i]),
                                 GUINT_TO_POINTER(TRUE)))
        {
            return FALSE;
        }
    }
    return TRUE;
}

gboolean kiran_menu_favorite_add_app(KiranMenuFavorite *self,
                                     const char *desktop_id)
{
    if (g_hash_table_lookup(self->favorite_apps, desktop_id) == NULL)
    {
        g_hash_table_insert(self->favorite_apps, g_strdup(desktop_id),
                            GUINT_TO_POINTER(TRUE));
        write_favorite_to_settings(self);
        return TRUE;
    }
    return FALSE;
}

gboolean kiran_menu_favorite_del_app(KiranMenuFavorite *self,
                                     const char *desktop_id)
{
    if (g_hash_table_lookup(self->favorite_apps, desktop_id) != NULL)
    {
        g_hash_table_remove(self->favorite_apps, desktop_id);
        write_favorite_to_settings(self);
        return TRUE;
    }
    return FALSE;
}

static gboolean deleted_app_callback(gpointer key, gpointer value,
                                     gpointer user_data)
{
    gchar *desktop_id = (gchar *)key;
    GQuark quark = g_quark_from_string(desktop_id);
    GHashTable *valid_apps = (GHashTable *)user_data;
    return (g_hash_table_lookup(valid_apps, GUINT_TO_POINTER(quark)) == NULL);
}

void kiran_menu_favorite_flush(KiranMenuFavorite *self, GList *apps)
{
    g_autoptr(GHashTable) app_table = g_hash_table_new(NULL, NULL);

    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        const gchar *desktop_id = kiran_app_get_desktop_id(app);
        GQuark quark = g_quark_from_string(desktop_id);
        g_hash_table_insert(app_table, GUINT_TO_POINTER(quark), GUINT_TO_POINTER(TRUE));
    }

    guint remove_num = g_hash_table_foreach_remove(self->favorite_apps, deleted_app_callback, app_table);
    if (remove_num > 0)
    {
        write_favorite_to_settings(self);
    }
}

GList *kiran_menu_favorite_get_favorite_apps(KiranMenuFavorite *self)
{
    GList *apps = NULL;
    GHashTableIter iter;
    gchar *desktop_id = NULL;
    guint *value = NULL;
    g_hash_table_iter_init(&iter, self->favorite_apps);
    while (g_hash_table_iter_next(&iter, (gpointer *)&desktop_id,
                                  (gpointer *)&value))
    {
        apps = g_list_append(apps, g_strdup(desktop_id));
    }
    return apps;
}

static void kiran_menu_favorite_init(KiranMenuFavorite *self)
{
    self->settings = g_settings_new(KIRAN_MENU_SCHEMA);
    read_favorite_from_settings(self);
}

static void kiran_menu_favorite_dispose(GObject *object)
{
    KiranMenuFavorite *menu_favorite = KIRAN_MENU_FAVORITE(object);

    g_clear_pointer(&menu_favorite->settings, g_object_unref);
    g_clear_pointer(&menu_favorite->favorite_apps, g_hash_table_unref);

    G_OBJECT_CLASS(kiran_menu_favorite_parent_class)->dispose(object);
}

static void kiran_menu_favorite_class_init(KiranMenuFavoriteClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_favorite_dispose;
}

KiranMenuFavorite *kiran_menu_favorite_get_new(void)
{
    return g_object_new(KIRAN_TYPE_MENU_FAVORITE, NULL);
}
