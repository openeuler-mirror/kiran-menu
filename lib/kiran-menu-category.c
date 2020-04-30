/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:42:17
 * @Description  :
 * @FilePath     : /kiran-menu-backend/lib/kiran-menu-category.c
 */
#include "lib/kiran-menu-category.h"

#include "lib/helper.h"
#include "lib/kiran-menu-app.h"

struct _KiranMenuCategory
{
    GObject parent;
    GHashTable *categorys;
};

G_DEFINE_TYPE(KiranMenuCategory, kiran_menu_category, G_TYPE_OBJECT)

static gboolean category_add_app(KiranMenuCategory *self, const char *category,
                                 KiranApp *app)
{
    g_return_val_if_fail(category != NULL, FALSE);

    GHashTable *category_info = g_hash_table_lookup(self->categorys, category);
    if (category_info == NULL)
    {
        category_info = g_hash_table_new(NULL, NULL);
        g_hash_table_insert(self->categorys, g_strdup(category), category_info);
    }

    const char *desktop_id = kiran_app_get_desktop_id(app);
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);
    return g_hash_table_insert(category_info, GUINT_TO_POINTER(quark),
                               GUINT_TO_POINTER(TRUE));
}

static gboolean category_del_app(KiranMenuCategory *self, const char *category,
                                 KiranApp *app)
{
    g_return_val_if_fail(category != NULL, FALSE);

    GHashTable *category_info = g_hash_table_lookup(self->categorys, category);
    RETURN_VAL_IF_TRUE(category_info == NULL, FALSE);

    const char *desktop_id = kiran_app_get_desktop_id(app);
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);
    return g_hash_table_remove(category_info, GUINT_TO_POINTER(quark));
}

// static gboolean category_contain_app(KiranMenuCategory *self,
//                                      const char *category,
//                                      const KiranApp *app) {
//   g_return_val_if_fail(category != NULL, FALSE);
//   CategoryInfo *category_info = g_hash_table_lookup(self->categorys,
//   category);

//   RETURN_VAL_IF_TRUE(category_info == NULL, FALSE)

//   const char *desktop_id = kiran_app_get_desktop_id(app);
//   g_return_val_if_fail(desktop_id != NULL, FALSE);

//   GQuark quark = g_quark_from_string(desktop_id);
//   return (g_hash_table_lookup(category_info, GUINT_TO_POINTER(quark)) !=
//   NULL);
// }

gboolean kiran_menu_category_load(KiranMenuCategory *self, GList *apps)
{
    if (self->categorys)
    {
        g_hash_table_destroy(self->categorys);
    }
    self->categorys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                            (GDestroyNotify)g_hash_table_unref);
    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        const char *categories = kiran_app_get_categories(app);

        CONTINUE_IF_TRUE(categories == NULL);

        g_auto(GStrv) strv = g_strsplit(categories, ";", -1);
        gint i = 0;
        for (; strv[i] != NULL; ++i)
        {
            if (strv[i][0] != '\0')
            {
                category_add_app(self, strv[i], app);
            }
        }
    }
    return TRUE;
}

gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(menu_app != NULL, FALSE);

    if (kiran_menu_app_add_category(menu_app, category) &&
        category_add_app(self, category, KIRAN_APP(menu_app)))
    {
        return TRUE;
    }
    return FALSE;
}

gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(menu_app != NULL, FALSE);

    if (category_del_app(self, category, KIRAN_APP(menu_app)) &&
        kiran_menu_app_del_category(menu_app, category))
    {
        return TRUE;
    }
    return FALSE;
}

GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category)
{
    g_return_val_if_fail(category != NULL, FALSE);
    GHashTable *category_info = g_hash_table_lookup(self->categorys, category);

    RETURN_VAL_IF_TRUE(category_info == NULL, NULL);

    GList *apps = NULL;
    GHashTableIter iter;
    gpointer key;
    gpointer value;
    g_hash_table_iter_init(&iter, category_info);
    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&value))
    {
        GQuark quark = GPOINTER_TO_UINT(key);
        const char *desktop_id = g_quark_to_string(quark);
        if (!desktop_id)
        {
            g_info("GQuark %u in category %s not be found \n", quark, category);
            continue;
        }
        apps = g_list_append(apps, g_strdup(desktop_id));
    }
    return apps;
}

GList *kiran_menu_category_get_categorys(KiranMenuCategory *self)
{
    GList *categorys = NULL;
    GHashTableIter iter;
    gchar *category;
    GHashTable *apps;
    g_hash_table_iter_init(&iter, self->categorys);
    while (
        g_hash_table_iter_next(&iter, (gpointer *)&category, (gpointer *)&apps))
    {
        categorys = g_list_append(categorys, g_strdup(category));
    }
    return categorys;
}

static void destory_apps_func(gpointer data)
{
    GList *apps = (GList *)data;
    g_list_free_full(apps, g_free);
}

GHashTable *kiran_menu_category_get_all(KiranMenuCategory *self)
{
    GHashTable *categorys =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, destory_apps_func);

    GHashTableIter iter;
    gchar *category;
    GHashTable *apps;
    g_hash_table_iter_init(&iter, self->categorys);
    while (
        g_hash_table_iter_next(&iter, (gpointer *)&category, (gpointer *)&apps))
    {
        GList *new_apps = kiran_menu_category_get_apps(self, category);
        g_hash_table_insert(categorys, g_strdup(category), new_apps);
    }
    return categorys;
}

static void kiran_menu_category_init(KiranMenuCategory *self) {}

static void kiran_menu_category_dispose(GObject *object)
{
    KiranMenuCategory *menu_category = KIRAN_MENU_CATEGORY(object);
    g_clear_pointer(&(menu_category->categorys), g_hash_table_unref);
    G_OBJECT_CLASS(kiran_menu_category_parent_class)->dispose(object);
}

static void kiran_menu_category_class_init(KiranMenuCategoryClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_category_dispose;
}

KiranMenuCategory *kiran_menu_category_get_new()
{
    return g_object_new(KIRAN_TYPE_MENU_CATEGORY, NULL);
}

KiranMenuCategory *kiran_menu_category_get_new_with_apps(GList *apps)
{
    KiranMenuCategory *menu_category = kiran_menu_category_get_new();
    if (menu_category)
    {
        kiran_menu_category_load(menu_category, apps);
    }
    return menu_category;
}