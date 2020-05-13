/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 19:44:16
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 14:08:18
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-usage.h
 */
#pragma once

#include <gio/gio.h>

#include "lib/kiran-menu-unit.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_USAGE (kiran_menu_usage_get_type())

G_DECLARE_FINAL_TYPE(KiranMenuUsage, kiran_menu_usage, KIRAN, MENU_USAGE, KiranMenuUnit)

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
KiranMenuUsage *kiran_menu_usage_get_new();

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_usage_get_nfrequent_apps(KiranMenuUsage *self, gint top_n);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_usage_reset(KiranMenuUsage *self);

G_END_DECLS
