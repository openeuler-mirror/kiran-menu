/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 16:04:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 11:43:54
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-common.h
 */

#pragma once

#include <gio/gio.h>

#define KIRAN_MENU_SCHEMA "com.unikylin.Kiran.StartMenu"

#define MENU_KEY_FAVORITE_APPS "favorite-apps"

#define MENU_KEY_FREQUENT_APPS "frequent-apps"

#define MENU_KEY_NEW_APPS "new-apps"

GList *read_as_to_list_quark(GSettings *settings, const gchar *key);

gboolean write_list_quark_to_as(GSettings *settings, const gchar *key, GList *value);