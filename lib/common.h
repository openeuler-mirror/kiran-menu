/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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

std::list<std::string> convert_chinese_characters_to_pinyin(const std::string &contents);
}  // namespace Kiran