/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 09:43:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 17:34:20
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category.c
 */

#include "lib/kiran-category.h"

struct _KiranCategory
{
    GObject parent_instance;

    gchar *name;
    gchar *icon;

    KiranCategoryNode *node;

    GHashTable *category_apps;
};

G_DEFINE_TYPE(KiranCategory, kiran_category, G_TYPE_OBJECT);

const gchar *kiran_category_get_name(KiranCategory *self)
{
    return self->name;
}

const gchar *kiran_category_get_icon(KiranCategory *self)
{
    return self->icon;
}

GList *kiran_category_get_apps(KiranCategory *self)
{
    GList *apps = NULL;
    GHashTableIter iter;
    gpointer key;
    gpointer value;
    g_hash_table_iter_init(&iter, self->category_apps);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GQuark quark = GPOINTER_TO_UINT(key);
        const gchar *desktop_id = g_quark_to_string(quark);
        apps = g_list_append(apps, g_strdup(desktop_id));
    }
    return apps;
}

static void kiran_category_add_app(KiranCategory *self, KiranApp *app)
{
    const gchar *desktop_id = kiran_app_get_desktop_id(app);
    GQuark quark = g_quark_from_string(desktop_id);
    g_hash_table_insert(self->category_apps, GUINT_TO_POINTER(quark), GUINT_TO_POINTER(TRUE));
}

static void kiran_category_del_app(KiranCategory *self, KiranApp *app)
{
    const gchar *desktop_id = kiran_app_get_desktop_id(app);
    GQuark quark = g_quark_from_string(desktop_id);
    g_hash_table_remove(self->category_apps, GUINT_TO_POINTER(quark));
}

static gboolean match_desktop_category(KiranCategoryNode *node, KiranApp *app)
{
    const gchar *categories = kiran_app_get_categories(app);
    const gchar *desktop_category = node->content;

    if (!categories)
    {
        return FALSE;
    }

    g_auto(GStrv) strv = g_strsplit(categories, ";", -1);
    for (gint i = 0; strv && strv[i] != NULL; ++i)
    {
        //g_print("%s:---%s---%s\n", kiran_app_get_desktop_id(app), strv[i], desktop_category);
        gsize len = strlen(desktop_category);
        if (g_ascii_strncasecmp(strv[i], desktop_category, len + 1) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean match_desktop_id(KiranCategoryNode *node, KiranApp *app)
{
    const gchar *app_desktop_id = kiran_app_get_desktop_id(app);
    const gchar *node_desktop_id = node->content;

    if (g_strcmp0(app_desktop_id, node_desktop_id) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static gboolean match_rule(KiranCategoryNode *node, KiranApp *app)
{
    gboolean match_result = FALSE;

    if (node->type == CATEGORY_NODE_TYPE_DESKTOP_CATEGORY)
    {
        return match_desktop_category(node, app);
    }
    else if (node->type == CATEGORY_NODE_TYPE_DESKTOP_ID)
    {
        return match_desktop_id(node, app);
    }
    else
    {
        KiranCategoryNode *iter = node->children;
        gboolean match_finish = FALSE;
        while (iter && !match_finish)
        {
            gboolean sub_result = match_rule(iter, app);
            // first match result
            if (iter == node->children)
            {
                match_result = sub_result;
            }

            switch (node->type)
            {
                case CATEGORY_NODE_TYPE_LOGIC:
                case CATEGORY_NODE_TYPE_OR:
                case CATEGORY_NODE_TYPE_INCLUDE:
                case CATEGORY_NODE_TYPE_EXCLUDE:
                    match_result = (match_result | sub_result);
                    if (match_result)
                    {
                        match_finish = TRUE;
                    }
                    break;
                case CATEGORY_NODE_TYPE_AND:
                    match_result = (match_result & sub_result);
                    if (!match_result)
                    {
                        match_finish = TRUE;
                    }
                    break;
                case CATEGORY_NODE_TYPE_NOT:
                    match_result = (!sub_result);
                    match_finish = TRUE;
                    break;
            }

            iter = (iter->next == node->children) ? NULL : iter->next;
        }
        return match_result;
    }
}

static gboolean match_app(KiranCategory *self, KiranApp *app)
{
    g_return_val_if_fail(self->node != NULL, FALSE);

    gboolean match_result = FALSE;
    KiranCategoryNode *iter = NULL;

    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_LOGIC ||
            iter->type == CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = TRUE;
                break;
            }
        }
    }

    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_EXCLUDE)
        {
            if (match_result && match_rule(iter, app))
            {
                match_result = FALSE;
                break;
            }
        }
    }
    return match_result;
}

gboolean kiran_category_match_add_app(KiranCategory *self, KiranApp *app)
{
    if (match_app(self, app))
    {
        kiran_category_add_app(self, app);
        return TRUE;
    }
    return FALSE;
}

static gboolean delete_node_with_desktop_id(KiranCategoryNode *node, const gchar *desktop_id)
{
    g_return_val_if_fail(node != NULL, FALSE);

    KiranCategoryNode *iter = NULL;

    iter = node->children;
    for (; iter; iter = (iter->next == node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_DESKTOP_ID &&
            g_strcmp0(desktop_id, iter->content) == 0)
        {
            kiran_category_node_steal(iter);
            g_object_unref(iter);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean kiran_category_add_rule_include_app(KiranCategory *self, KiranApp *app)
{
    g_return_val_if_fail(self->node != NULL, FALSE);

    KiranCategoryNode *iter = NULL;

    gboolean rule_change = FALSE;

    // remove desktop_id from <exclude> element
    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_EXCLUDE &&
            match_rule(iter, app))
        {
            const gchar *desktop_id = kiran_app_get_desktop_id(app);
            delete_node_with_desktop_id(iter, desktop_id);
            if (iter->children == NULL)
            {
                kiran_category_node_steal(iter);
                g_object_unref(iter);
            }
            rule_change = TRUE;
            break;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    gboolean match_result = FALSE;
    KiranCategoryNode *include_node = NULL;
    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_INCLUDE)
        {
            include_node = iter;
        }

        if (iter->type == CATEGORY_NODE_TYPE_LOGIC ||
            iter->type == CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = TRUE;
                break;
            }
        }
    }

    // add desktop_id to <include> element if match_result equal to false
    if (!match_result)
    {
        if (!include_node)
        {
            include_node = kiran_category_node_get_new(CATEGORY_NODE_TYPE_INCLUDE);
            kiran_category_node_append_child(self->node, include_node);
        }
        KiranCategoryNode *desktop_id_node = kiran_category_node_get_new(CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->content = g_strdup(kiran_app_get_desktop_id(app));
        kiran_category_node_append_child(include_node, desktop_id_node);
        rule_change = TRUE;
    }

    if (rule_change)
    {
        kiran_category_add_app(self, app);
    }

    return rule_change;
}

gboolean kiran_category_add_rule_exclude_app(KiranCategory *self, KiranApp *app)
{
    g_return_val_if_fail(self->node != NULL, FALSE);

    KiranCategoryNode *iter = NULL;
    KiranCategoryNode *exclude_node = NULL;

    // check whether desktop_id exist in <exclude> element
    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_EXCLUDE)
        {
            exclude_node = iter;
        }

        if (iter->type == CATEGORY_NODE_TYPE_EXCLUDE &&
            match_rule(iter, app))
        {
            return FALSE;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    gboolean match_result = FALSE;
    iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_LOGIC ||
            iter->type == CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = TRUE;
                break;
            }
        }
    }

    // add desktop_id to <exclude> element if match_result equal to true
    if (match_result)
    {
        if (!exclude_node)
        {
            exclude_node = kiran_category_node_get_new(CATEGORY_NODE_TYPE_EXCLUDE);
            kiran_category_node_append_child(self->node, exclude_node);
        }
        KiranCategoryNode *desktop_id_node = kiran_category_node_get_new(CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->content = g_strdup(kiran_app_get_desktop_id(app));
        kiran_category_node_append_child(exclude_node, desktop_id_node);

        kiran_category_del_app(self, app);
        return TRUE;
    }
    return FALSE;
}

static void kiran_category_init(KiranCategory *self)
{
    self->name = NULL;
    self->icon = NULL;

    self->node = NULL;

    self->category_apps = g_hash_table_new(NULL, NULL);
}

static void kiran_category_dispose(GObject *object)
{
    KiranCategory *self = KIRAN_CATEGORY(object);

    g_clear_pointer(&(self->name), g_free);
    g_clear_pointer(&(self->icon), g_free);
    g_clear_pointer(&(self->node), g_object_unref);
    g_clear_pointer(&(self->category_apps), g_hash_table_unref);

    G_OBJECT_CLASS(kiran_category_parent_class)->dispose(object);
}

static void kiran_category_class_init(KiranCategoryClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_category_dispose;
}

static gboolean _kiran_category_init(KiranCategory *self, KiranCategoryNode *node)
{
    g_return_val_if_fail(node != NULL, FALSE);

    if (node->type != CATEGORY_NODE_TYPE_CATEGORY)
    {
        g_warning("KiranCategory init need the node which type must be CATEGORY_NODE_TYPE_CATEGORY.");
        return FALSE;
    }

    self->node = g_object_ref(node);

    KiranCategoryNode *iter = self->node->children;
    for (; iter; iter = (iter->next == self->node->children) ? NULL : iter->next)
    {
        if (iter->type == CATEGORY_NODE_TYPE_NAME)
        {
            self->name = g_strdup(iter->content);
        }
        else if (iter->type == CATEGORY_NODE_TYPE_ICON)
        {
            self->icon = g_strdup(iter->content);
        }
    }

    return TRUE;
}

KiranCategory *kiran_category_get_new(KiranCategoryNode *node)
{
    KiranCategory *category = g_object_new(KIRAN_TYPE_CATEGORY, NULL);
    if (category && !_kiran_category_init(category, node))
    {
        g_object_unref(category);
        return NULL;
    }
    return category;
}