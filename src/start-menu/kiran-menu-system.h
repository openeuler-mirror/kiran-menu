/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 22:22:05
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-system.h
 */
#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-menu-app.h"

#define KIRAN_TYPE_MENU_SYSTEM (kiran_menu_system_get_type())

G_DECLARE_FINAL_TYPE(KiranMenuSystem, kiran_menu_system, KIRAN, MENU_SYSTEM,
                     GObject)

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_apps(KiranMenuSystem *self);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
KiranMenuApp *kiran_menu_system_lookup_app(KiranMenuSystem *self,
                                           const char *app_id);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_nnew_apps(KiranMenuSystem *self, gint top_n);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_all_sorted_apps(KiranMenuSystem *self);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
KiranMenuSystem *kiran_menu_system_get_new(void);
