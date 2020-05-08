/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 09:43:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-08 11:26:51
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category.h
 */

#pragma once

#include <gio/gio.h>

#include "lib/kiran-app.h"
#include "lib/kiran-category-node.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_CATEGORY (kiran_category_get_type())
G_DECLARE_FINAL_TYPE(KiranCategory, kiran_category, KIRAN, CATEGORY, GObject);

KiranCategory *kiran_category_get_new(KiranCategoryNode *node);

const gchar *kiran_category_get_name(KiranCategory *self);

const gchar *kiran_category_get_icon(KiranCategory *self);

gboolean kiran_category_get_repeat(KiranCategory *self);

GList *kiran_category_get_apps(KiranCategory *self);

gboolean kiran_category_match_add_app(KiranCategory *self, KiranApp *app);

gboolean kiran_category_add_rule_include_app(KiranCategory *self, KiranApp *app);

gboolean kiran_category_add_rule_exclude_app(KiranCategory *self, KiranApp *app);

G_END_DECLS