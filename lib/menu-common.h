/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 16:04:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-03 17:58:56
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-common.h
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
#define KIRAN_MENU_SCHEMA "com.unikylin.Kiran.StartMenu"

#define MENU_KEY_FAVORITE_APPS "favorite-apps"

#define MENU_KEY_FREQUENT_APPS "frequent-apps"

#define MENU_KEY_NEW_APPS "new-apps"

std::list<int32_t> read_as_to_list_quark(Glib::RefPtr<Gio::Settings> settings,
                                         const std::string &key);

bool write_list_quark_to_as(Glib::RefPtr<Gio::Settings> settings,
                            const std::string &key,
                            const std::list<int32_t> &value);

}  // namespace Kiran