/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:30:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:40:53
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
 * @description: 创建KiranMenuCategory对象,
 * 并用apps进行初始化（调用kiran_menu_category_load函数）
 * @return:
 * @author: tangjie02
 */
KiranMenuCategory *kiran_menu_category_get_new_with_apps(GList *apps);

/**
 * @description: 通过app集合来初始化app分类
 * @param {KiranMenuCategory*} self KiranMenuCategory对象
 * @param {const GList*} apps 链表元素类型为KiranMenuApp*
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_load(KiranMenuCategory *self, GList *apps);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category,
                                     KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category,
                                     KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category);

/**
 * @description: 获取所有分类
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_category_get_categorys(KiranMenuCategory *self);

/**
 * @description: 获取所有分类和APP
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GHashTable *kiran_menu_category_get_all(KiranMenuCategory *self);

G_END_DECLS
