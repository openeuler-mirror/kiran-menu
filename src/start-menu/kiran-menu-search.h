/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 20:58:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 15:52:23
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-search.h
 */
#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_SEARCH (kiran_app_search_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuSearch, kiran_app_search, KIRAN, MENU_SEARCH,
                     GObject)

KiranMenuSearch *kiran_app_search_get_new();

GList *kiran_menu_search_by_keyword(KiranMenuSearch *self, const char *keyword,
                                    const GList *apps);

G_END_DECLS
