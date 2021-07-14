/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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