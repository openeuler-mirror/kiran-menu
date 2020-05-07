/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:29:00
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 14:13:36
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-node.h
 */

#pragma once

#include <gio/gio.h>

#define KIRAN_TYPE_CATEGORY_NODE (kiran_category_node_get_type())

G_DECLARE_FINAL_TYPE(KiranCategoryNode, kiran_category_node, KIRAN,
                     CATEGORY_NODE, GObject);

typedef enum
{
    CATEGORY_NODE_TYPE_UNKNOWN,
    CATEGORY_NODE_TYPE_ROOT,
    CATEGORY_NODE_TYPE_CATEGORIES,
    CATEGORY_NODE_TYPE_CATEGORY,
    CATEGORY_NODE_TYPE_NAME,
    CATEGORY_NODE_TYPE_ICON,
    CATEGORY_NODE_TYPE_LOGIC,
    CATEGORY_NODE_TYPE_AND,
    CATEGORY_NODE_TYPE_OR,
    CATEGORY_NODE_TYPE_NOT,
    CATEGORY_NODE_TYPE_DESKTOP_ID,
    CATEGORY_NODE_TYPE_DESKTOP_CATEGORY,
    CATEGORY_NODE_TYPE_INCLUDE,
    CATEGORY_NODE_TYPE_EXCLUDE,
} CategoryNodeType;

struct _KiranCategoryNode
{
    GObject parent_instance;

    KiranCategoryNode *prev;
    KiranCategoryNode *next;
    KiranCategoryNode *parent;
    KiranCategoryNode *children;

    gchar *content;

    guint type;
};

KiranCategoryNode *kiran_category_node_get_new(CategoryNodeType type);

void kiran_category_node_append_child(KiranCategoryNode *node, KiranCategoryNode *sub_node);

void kiran_category_node_steal(KiranCategoryNode *node);