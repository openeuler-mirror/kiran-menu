/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:30:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 18:06:22
 * @Description  : 管理菜单中的APP的分类
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-category.h
 */
#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-menu-system.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_CATEGORY (kiran_menu_category_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuCategory, kiran_menu_category, KIRAN,
                     MENU_CATEGORY, GObject)

/**
 * @description: 创建KiranMenuCategory对象
 * @return:
 * @author: tangjie02
 */
KiranMenuCategory *kiran_menu_category_get_new();

/**
 * @description: 通过app集合来初始化app分类
 * @param {KiranMenuCategory*} self KiranMenuCategory对象
 * @param {const GList*} apps 链表元素类型为KiranMenuApp*
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_load(KiranMenuCategory *self, const GList *apps);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category,
                                     const KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category,
                                     const KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GHashTable *kiran_menu_category_get_all(KiranMenuCategory *self);
G_END_DECLS
