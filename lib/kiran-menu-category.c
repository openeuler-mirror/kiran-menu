/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-08 11:25:50
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

    GList *categories;

    KiranCategoryNode *root;
};

G_DEFINE_TYPE(KiranMenuCategory, kiran_menu_category, G_TYPE_OBJECT)

void kiran_menu_category_load(KiranMenuCategory *self, GList *apps)
{
    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;

        gboolean match_result = FALSE;

        for (GList *l2 = self->categories; l2 != NULL; l2 = l2->next)
        {
            KiranCategory *category = l2->data;

            if (match_result && !kiran_category_get_repeat(category))
            {
                continue;
            }

            if (kiran_category_match_add_app(category, app))
            {
                match_result = TRUE;
            }
        }
    }
}

static void store_categories(KiranMenuCategory *self)
{
    KiranCategoryWriter *writer = kiran_category_writer_get_new();
    kiran_category_writer_to_xml(writer, self->root, self->file_path);
}

static KiranCategory *find_category(KiranMenuCategory *self,
                                    const char *category_name)
{
    for (GList *l = self->categories; l != NULL; l = l->next)
    {
        KiranCategory *category = l->data;
        const gchar *name = kiran_category_get_name(category);
        if (g_strcmp0(name, category_name) == 0)
        {
            return category;
        }
    }
    return NULL;
}

gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(category_name != NULL, FALSE);
    g_return_val_if_fail(menu_app != NULL, FALSE);

    KiranCategory *category = find_category(self, category_name);

    if (category)
    {
        if (kiran_category_add_rule_include_app(category, KIRAN_APP(menu_app)))
        {
            store_categories(self);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app)
{
    g_return_val_if_fail(category_name != NULL, FALSE);
    g_return_val_if_fail(menu_app != NULL, FALSE);

    KiranCategory *category = find_category(self, category_name);

    if (category)
    {
        if (kiran_category_add_rule_exclude_app(category, KIRAN_APP(menu_app)))
        {
            store_categories(self);
            return TRUE;
        }
    }
    return FALSE;
}

GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category_name)
{
    g_return_val_if_fail(category_name != NULL, NULL);

    KiranCategory *category = find_category(self, category_name);

    if (category)
    {
        return kiran_category_get_apps(category);
    }
    return NULL;
}

GList *kiran_menu_category_get_categorys(KiranMenuCategory *self)
{
    GList *category_names = NULL;

    for (GList *l = self->categories; l != NULL; l = l->next)
    {
        KiranCategory *category = l->data;
        const gchar *category_name = kiran_category_get_name(category);

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

    gchar *category_name;
    KiranCategory *category;

    for (GList *l = self->categories; l != NULL; l = l->next)
    {
        KiranCategory *category = l->data;
        const gchar *category_name = kiran_category_get_name(category);

        GList *new_apps = kiran_category_get_apps(category);

        g_hash_table_insert(categorys, g_strdup(category_name), new_apps);
    }
    return categorys;
}

static void kiran_menu_category_init(KiranMenuCategory *self)
{
    self->categories = NULL;

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
                if (find_category(self, name) != NULL)
                {
                    g_warning("Multiple category exist same name: %s\n", name);
                    g_object_unref(category);
                    continue;
                }
                self->categories = g_list_append(self->categories, category);
            }
        }
    }
    g_object_unref(reader);
}

static void kiran_menu_category_dispose(GObject *object)
{
    KiranMenuCategory *self = KIRAN_MENU_CATEGORY(object);

    g_clear_pointer(&(self->file_path), g_free);
    g_clear_pointer(&(self->root), g_object_unref);

    g_list_free_full(self->categories, g_object_unref);
    self->categories = NULL;

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