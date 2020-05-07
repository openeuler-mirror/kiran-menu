/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:29:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 16:56:11
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-node.c
 */

#include "lib/kiran-category-node.h"

G_DEFINE_TYPE(KiranCategoryNode, kiran_category_node, G_TYPE_OBJECT);

static void kiran_category_node_insert_after(KiranCategoryNode *node,
                                             KiranCategoryNode *new_sibling)
{
    g_return_if_fail(new_sibling != NULL);
    g_return_if_fail(new_sibling->parent == NULL);

    g_return_if_fail(node->parent != NULL);

    new_sibling->prev = node;
    new_sibling->next = node->next;

    node->next = new_sibling;
    new_sibling->next->prev = new_sibling;

    new_sibling->parent = node->parent;
}

void kiran_category_node_append_child(KiranCategoryNode *node, KiranCategoryNode *sub_node)
{
    if (node->children)
    {
        kiran_category_node_insert_after(node->children->prev, sub_node);
    }
    else
    {
        node->children = sub_node;
        sub_node->parent = node;
    }
}

void kiran_category_node_steal(KiranCategoryNode *node)
{
    if (node->parent && node->parent->children == node)
    {
        node->parent->children = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    node->next = node;
    node->prev = node;
    node->parent = NULL;
}

static void kiran_category_node_init(KiranCategoryNode *self)
{
    self->prev = self;
    self->next = self;
    self->parent = NULL;
    self->children = NULL;

    self->content = NULL;

    self->type = CATEGORY_NODE_TYPE_UNKNOWN;
}

static void kiran_category_node_dispose(GObject *object)
{
    KiranCategoryNode *self = KIRAN_CATEGORY_NODE(object);

    self->prev = self;
    self->next = self;
    self->parent = NULL;

    g_clear_pointer(&(self->content), g_free);

    KiranCategoryNode *iter = self->children;
    while (iter)
    {
        KiranCategoryNode *tmp = iter;
        iter = (iter->next == self->children) ? NULL : iter->next;
        g_object_unref(tmp);
    }

    self->type = CATEGORY_NODE_TYPE_UNKNOWN;

    G_OBJECT_CLASS(kiran_category_node_parent_class)->dispose(object);
}

static void kiran_category_node_class_init(KiranCategoryNodeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_category_node_dispose;
}

KiranCategoryNode *kiran_category_node_get_new(CategoryNodeType type)
{
    KiranCategoryNode *node = g_object_new(KIRAN_TYPE_CATEGORY_NODE, NULL);
    node->type = type;
    return node;
}