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
#define STARTMENU_SCHEMA "com.kylinsec.kiran.startmenu"
#define STARTMENU_KEY_FAVORITE_APPS "favorite-apps"
#define STARTMENU_KEY_FREQUENT_APPS "frequent-apps"
#define STARTMENU_KEY_NEW_APPS "new-apps"

#define TASKBAR_SCHEMA "com.kylinsec.kiran.taskbar"
#define TASKBAR_KEY_FIXED_APPS "fixed-apps"
#define TASKBAR_KEY_SHOW_ACTIVE_WORKSPACE "show-active-workspace-apps"
#define TASKBAR_KEY_SIMPLY_WINDOW_PREVIEWER "simply-window-previewer"

#define STARTMENU_LOCKDOWN_SCHEMA "com.kylinsec.kiran.startmenu.lockdown"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOCK_SCREEN "disable-lock-screen"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_USER_SWITCHING "disable-user-switching"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOG_OUT "disable-log-out"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SUSPEND "disable-suspend"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_HIBERNATE "disable-hibernate"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_REBOOT "disable-reboot"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SHUTDOWN "disable-shutdown"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_FIXED_APP "disable-fixed-app"

#define WORKSPACE_SCHEMA "com.kylinsec.kiran.workspace-switcher"
#define WORKSPACE_KEY_DRAW_WINDOWS "draw-windows-in-thumbnails"

#define STARTMENU_PROFILE_SCHEMA "com.kylinsec.kiran.startmenu.profile"
#define STARTMENU_PROFILE_KEY_DEFAULT_PAGE "default-page"
#define STARTMENU_PROFILE_KEY_DISPLAY_MODE "display-mode"
#define STARTMENU_PROFILE_KEY_WINDOW_OPACITY "background-opacity"

std::list<int32_t>
read_as_to_list_quark(Glib::RefPtr<Gio::Settings> settings,
                      const std::string &key);

bool write_list_quark_to_as(Glib::RefPtr<Gio::Settings> settings,
                            const std::string &key,
                            const std::list<int32_t> &value);

std::list<std::string> convert_chinese_characters_to_pinyin(const std::string &contents);
}  // namespace Kiran