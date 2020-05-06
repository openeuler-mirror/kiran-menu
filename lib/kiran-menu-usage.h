/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 19:44:16
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 22:17:33
 * @Description  :
 * @FilePath     : /kiran-menu-backend/lib/kiran-menu-usage.h
 */
#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_USAGE (kiran_menu_usage_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuUsage, kiran_menu_usage, KIRAN, MENU_USAGE,
                     GObject)

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
gboolean kiran_menu_usage_focus_app(KiranMenuUsage *self,
                                    const char *desktop_id);
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
