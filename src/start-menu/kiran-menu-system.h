/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 17:22:53
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-system.h
 */
#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-app-info.h"

#define KIRAN_TYPE_MENU_SYSTEM (kiran_menu_system_get_type())

G_DECLARE_FINAL_TYPE(KiranMenuSystem, kiran_menu_system, KIRAN, MENU_SYSTEM,
                     GObject)

GList *kiran_menu_system_get_apps(KiranMenuSystem *self);

KiranAppInfo *kiran_menu_system_lookup_app(KiranMenuSystem *self,
                                           const char *app_id);

gchar **kiran_menu_system_get_all_sorted_apps(KiranMenuSystem *self);

KiranMenuSystem *kiran_menu_system_get_new(void);
