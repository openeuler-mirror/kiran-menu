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

static gboolean write_favorite_to_settings(KiranMenuFavorite *self)
{
    GArray *apps = g_array_new(FALSE, FALSE, sizeof(gchar *));

    for (GList *l = self->favorite_apps; l != NULL; l = l->next)
    {
        GQuark quark = GPOINTER_TO_UINT(l->data);
        const gchar *desktop_id = g_quark_to_string(quark);
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
    g_clear_pointer(&self->favorite_apps, g_list_free);
    self->favorite_apps = NULL;

    g_auto(GStrv) favorite_apps = g_settings_get_strv(self->settings, "favorite-apps");

    for (gint i = 0; favorite_apps[i] != NULL; ++i)
    {
        GQuark quark = g_quark_from_string(favorite_apps[i]);
        self->favorite_apps = g_list_append(self->favorite_apps, GUINT_TO_POINTER(quark));
    }
    return TRUE;
}

gboolean kiran_menu_favorite_add_app(KiranMenuFavorite *self,
                                     const char *desktop_id)
{
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);

    if (g_list_find(self->favorite_apps, GUINT_TO_POINTER(quark)) == NULL)
    {
        self->favorite_apps = g_list_append(self->favorite_apps, GUINT_TO_POINTER(quark));
        write_favorite_to_settings(self);
        return TRUE;
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
        write_favorite_to_settings(self);
        return TRUE;
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

static gboolean deleted_app_callback(gpointer key, gpointer value,
                                     gpointer user_data)
{
    gchar *desktop_id = (gchar *)key;
    GQuark quark = g_quark_from_string(desktop_id);
    GHashTable *valid_apps = (GHashTable *)user_data;
    return (g_hash_table_lookup(valid_apps, GUINT_TO_POINTER(quark)) == NULL);
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

    gboolean have_remove = FALSE;
    GList *iter = self->favorite_apps;
    while (iter)
    {
        if (g_hash_table_lookup(app_table, iter->data) == NULL)
        {
            GList *tmp = iter;
            iter = iter->next;
            self->favorite_apps = g_list_delete_link(self->favorite_apps, tmp);
            have_remove = TRUE;
        }
        else
        {
            iter = iter->next;
        }
    }

    if (have_remove > 0)
    {
        write_favorite_to_settings(self);
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

static void kiran_menu_favorite_init(KiranMenuFavorite *self)
{
    self->settings = g_settings_new(KIRAN_MENU_SCHEMA);
    read_favorite_from_settings(self);
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

    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_favorite_dispose;
}

KiranMenuFavorite *kiran_menu_favorite_get_new(void)
{
    return g_object_new(KIRAN_TYPE_MENU_FAVORITE, NULL);
}
