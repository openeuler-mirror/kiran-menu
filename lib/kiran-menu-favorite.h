/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 15:37:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 17:04:18
 * @Description  :
 * @FilePath     : /kiran-menu-backend/lib/kiran-menu-favorite.h
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
 * @description: 如果收藏的APP不在valid_apps中, 则进行删除
 * @param {KiranMenuFavorite*} self KiranMenuFavorite对象
 * @param {const GHashTable*} valid_apps 合法的app, key应该为desktop_id.
 * @return: 如果清理失败则返回FALSE, 否则返回TRUE.
 * @author: tangjie02
 */
gboolean kiran_menu_favorite_clear_deleted_app(KiranMenuFavorite *self,
                                               const GHashTable *valid_apps);

/**
 * @description: 获取收藏的APP列表
 * @param {KiranMenuFavorite *} self KiranMenuFavorite对象
 * @return: 链表的元素类型为gchar*, 表示APP的desktop_id
 * @author: tangjie02
 */
GList *kiran_menu_favorite_get_favorite_apps(KiranMenuFavorite *self);

G_END_DECLS
