/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 00:43:04
 * @Description  : 维护APP的一些基本信息
 * @FilePath     : /kiran-menu-backend/lib/start-menu/kiran-app.h
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP (kiran_app_get_type())
G_DECLARE_DERIVABLE_TYPE(KiranApp, kiran_app, KIRAN, APP, GObject);

struct _KiranAppClass
{
    GObjectClass parent_class;

    gpointer padding[12];
};

/**
 * @description: 获取App的名字
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_name(KiranApp *self);

/**
 * @description: 获取App的locale_name
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_locale_name(KiranApp *self);

/**
 * @description:
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_comment(KiranApp *self);

/**
 * @description:
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_locale_comment(KiranApp *self);

/**
 * @description: 获取App的desktop_id
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_desktop_id(KiranApp *self);

/**
 * @description: 获取App的分类
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_categories(KiranApp *self);

/**
 * @description:
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
const gchar *kiran_app_get_file_name(KiranApp *self);

/**
 * @description:
 * @param {KiranApp*}
 * @return:
 * @author: tangjie02
 */
guint64 kiran_app_get_create_time(KiranApp *self);

/**
 * @description: 获取指定app的图标
 * @param {KiranApp*} 要获取图标的app
 * @return: 图标信息
 * @author: songchuanfei
 */
GIcon *kiran_app_get_icon(KiranApp *self);

/**
 * @description: 启动指定的app
 * @param {KiranApp*} 要启动的app
 * @return: 启动是否成功，成功返回TRUE，失败返回FALSE
 * @author: songchuanfei
 */
gboolean kiran_app_launch(KiranApp *self);

G_END_DECLS
