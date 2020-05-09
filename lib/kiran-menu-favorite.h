/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 15:37:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 16:16:44
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-favorite.h
 */
#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_FAVORITE (kiran_menu_favorite_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuFavorite, kiran_menu_favorite, KIRAN,
                     MENU_FAVORITE, GObject)

/**
 * @description:
 * @return:
 * @author: tangjie02
 */
KiranMenuFavorite *kiran_menu_favorite_get_new(void);

/**
 * @description: 添加收藏APP
 * @param {KiranMenuFavorite*}
 * @param {const char*}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_favorite_add_app(KiranMenuFavorite *self,
                                     const char *desktop_id);

/**
 * @description: 删除收藏APP
 * @param {KiranMenuFavorite*}
 * @param {const char*}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_favorite_del_app(KiranMenuFavorite *self,
                                     const char *desktop_id);

/**
 * @description: 查找收藏APP
 * @param {type} 
 * @return: 
 * @author: tangjie02
 */
gboolean kiran_menu_favorite_find_app(KiranMenuFavorite *self,
                                      const char *desktop_id);

/**
 * @description: 
 * @param {type} 
 * @return: 
 * @author: tangjie02
 */
void kiran_menu_favorite_flush(KiranMenuFavorite *self, GList *apps);

/**
 * @description: 获取收藏的APP列表
 * @param {KiranMenuFavorite *} self KiranMenuFavorite对象
 * @return: 链表的元素类型为gchar*, 表示APP的desktop_id
 * @author: tangjie02
 */
GList *kiran_menu_favorite_get_favorite_apps(KiranMenuFavorite *self);

G_END_DECLS
