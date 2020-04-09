/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:22:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:47:30
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-app.h
 */
#pragma once

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include "src/start-menu/kiran-app.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_APP (kiran_menu_app_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuApp, kiran_menu_app, KIRAN, MENU_APP, KiranApp);

gboolean kiran_menu_app_add_category(KiranMenuApp *self, const char *category);
gboolean kiran_menu_app_del_category(KiranMenuApp *self, const char *category);

KiranMenuApp *kiran_menu_app_get_new(const char *desktop_id);

G_END_DECLS
