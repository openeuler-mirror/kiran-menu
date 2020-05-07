/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 18:03:11
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-category.c
 */
#include "lib/kiran-menu-category.h"

#include "lib/helper.h"
#include "lib/kiran-category-reader.h"
#include "lib/kiran-category-writer.h"
#include "lib/kiran-category.h"
#include "lib/kiran-menu-app.h"

struct _KiranMenuCategory
{
    GObject parent;

    gchar *file_path;
    GHashTable *categories;
    KiranCategoryNode *root;
};

G_DEFINE_TYPE(KiranMenuCategory, kiran_menu_category, G_TYPE_OBJECT)

static gboolean category_add_app(KiranMenuCategory *self, const char *category,
                                 KiranApp *app)
{
    g_return_val_if_fail(category != NULL, FALSE);

    GHashTable *category_info = g_hash_table_lookup(self->categories, category);
    if (category_info == NULL)
    {
        category_info = g_hash_table_new(NULL, NULL);
        g_hash_table_insert(self->categories, g_strdup(category), category_info);
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

    GHashTable *category_info = g_hash_table_lookup(self->categories, category);
    RETURN_VAL_IF_TRUE(category_info == NULL, FALSE);

    const char *desktop_id = kiran_app_get_desktop_id(app);
    g_return_val_if_fail(desktop_id != NULL, FALSE);

    GQuark quark = g_quark_from_string(desktop_id);
    return g_hash_table_remove(category_info, GUINT_TO_POINTER(quark));
}

void kiran_menu_category_load(KiranMenuCategory *self, GList *apps)
{
    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        GHashTableIter iter;
        gchar *name;
        KiranCategory *category;

        g_hash_table_iter_init(&iter, self->categories);
        while (g_hash_table_iter_next(&iter, (gpointer *)&name, (gpointer *)&category))
        {
            kiran_category_match_add_app(category, app);
        }
    }
}

static void kiran_menu_category_store(KiranMenuCategory *self)
{
    KiranCategoryWriter *writer = kiran_category_writer_get_new();
    kiran_category_writer_to_xml(writer, self->root, self->file_path);
}

gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(category_name != NULL, FALSE);
    g_return_val_if_fail(menu_app != NULL, FALSE);

    KiranCategory *category = g_hash_table_lookup(self->categories, category_name);
    if (category && kiran_category_add_rule_include_app(category, KIRAN_APP(menu_app)))
    {
        kiran_menu_category_store(self);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(category_name != NULL, FALSE);
    g_return_val_if_fail(menu_app != NULL, FALSE);

    KiranCategory *category = g_hash_table_lookup(self->categories, category_name);
    if (category && kiran_category_add_rule_exclude_app(category, KIRAN_APP(menu_app)))
    {
        kiran_menu_category_store(self);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category_name)
{
    g_return_val_if_fail(category_name != NULL, NULL);

    KiranCategory *category = g_hash_table_lookup(self->categories, category_name);

    if (category)
    {
        return kiran_category_get_apps(category);
    }
    return NULL;
}

GList *kiran_menu_category_get_categorys(KiranMenuCategory *self)
{
    GList *category_names = NULL;
    GHashTableIter iter;
    gchar *category_name;
    KiranCategory *category;
    g_hash_table_iter_init(&iter, self->categories);
    while (
        g_hash_table_iter_next(&iter, (gpointer *)&category_name, (gpointer *)&category))
    {
        category_names = g_list_append(category_names, g_strdup(category_name));
    }
    return category_names;
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
    gchar *category_name;
    KiranCategory *category;
    g_hash_table_iter_init(&iter, self->categories);
    while (g_hash_table_iter_next(&iter, (gpointer *)&category_name, (gpointer *)&category))
    {
        GList *new_apps = kiran_category_get_apps(category);
        g_hash_table_insert(categorys, g_strdup(category_name), new_apps);
    }
    return categorys;
}

static void kiran_menu_category_init(KiranMenuCategory *self)
{
    self->categories = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                             (GDestroyNotify)g_object_unref);

    self->file_path = g_build_filename("/usr",
                                       "share",
                                       "kiran-menu",
                                       "com.unikylin.Kiran.MenuCategory.xml",
                                       NULL);

    KiranCategoryReader *reader = kiran_category_reader_get_new();

    self->root = kiran_category_reader_from_xml(reader, self->file_path);

    if (self->root != NULL &&
        self->root->children != NULL)
    {
        KiranCategoryNode *iter = self->root->children->children;
        for (; iter; iter = (iter->next == self->root->children->children) ? NULL : iter->next)
        {
            if (iter->type != CATEGORY_NODE_TYPE_CATEGORY)
            {
                g_warning("exist invalid node type: %u\n", iter->type);
                continue;
            }
            KiranCategory *category = kiran_category_get_new(iter);
            if (category)
            {
                const gchar *name = kiran_category_get_name(category);
                KiranCategory *value = g_hash_table_lookup(self->categories, name);
                if (value)
                {
                    g_warning("Multiple category exist same name: %s\n", name);
                    g_object_unref(category);
                    continue;
                }
                g_hash_table_insert(self->categories, g_strdup(name), category);
            }
        }
    }
    g_object_unref(reader);
}

static void kiran_menu_category_dispose(GObject *object)
{
    KiranMenuCategory *menu_category = KIRAN_MENU_CATEGORY(object);

    g_clear_pointer(&(menu_category->file_path), g_free);
    g_clear_pointer(&(menu_category->categories), g_hash_table_unref);
    g_clear_pointer(&(menu_category->root), g_object_unref);

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