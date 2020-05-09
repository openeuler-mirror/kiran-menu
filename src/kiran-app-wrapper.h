#ifndef KIARN_APP_WRAPPER_INCLUDE_H
#define KIRAN_APP_WRAPPER_INCLUDE_H

#include <kiran-menu-app.h>
#include <kiran-menu-based.h>

/**
 * 检查给定的app是否位于收藏夹中
 * 
 * @param app 待检查的app
 *
 * @return 在收藏夹中返回TRUE，不在收藏夹中返回FALSE
 */
gboolean kiran_app_is_favorite(KiranApp *app);

/**
 * 将给定app添加到收藏夹中
 *
 * @param app 待检查的app
 * 
 * @return 如果app已经在收藏夹中返回TRUE，否则添加成功返回TRUE，添加失败返回FALSE
 */
gboolean kiran_app_add_to_favorite(KiranApp *app);

/**
 * 将给定app从收藏夹中删除
 * 
 * @param app 待检查的app
 * @return 如果app不在收藏夹中或删除成功，返回TRUE, 否则返回FALSE
 */
gboolean kiran_app_remove_from_favorite(KiranApp *app);

/**
 * 将指定的app添加到桌面
 * 
 * @param app 待添加的app
 * @return 如果app添加成功返回TRUE, 否则返回FALSE
 */
gboolean kiran_app_add_to_desktop(KiranApp *app);

#endif    //KIRAN_APP_WRAPPER_INCLUDE_H