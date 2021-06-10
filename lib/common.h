/**
 * @file          /kiran-cc-daemon/home/tangjie02/git/kiran-menu/lib/common.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
#define KIRAN_MENU_SCHEMA "com.kylinsec.kiran.startmenu"

#define MENU_KEY_FAVORITE_APPS "favorite-apps"

#define MENU_KEY_FREQUENT_APPS "frequent-apps"

#define MENU_KEY_NEW_APPS "new-apps"

#define KIRAN_TASKBAR_SCHEMA "com.kylinsec.kiran.taskbar"

#define TASKBAR_KEY_FIXED_APPS "fixed-apps"
#define TASKBAR_KEY_SHOW_ACTIVE_WORKSPACE "show-active-workspace-apps"

std::list<int32_t> read_as_to_list_quark(Glib::RefPtr<Gio::Settings> settings,
                                         const std::string &key);

bool write_list_quark_to_as(Glib::RefPtr<Gio::Settings> settings,
                            const std::string &key,
                            const std::list<int32_t> &value);

}  // namespace Kiran