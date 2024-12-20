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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "menu-profile.h"
#include "lib/base.h"
#include "lib/common.h"
MenuProfile::MenuProfile() : settings(Gio::Settings::create(STARTMENU_PROFILE_SCHEMA))
{
    settings->signal_changed().connect(
        sigc::mem_fun(*this, &MenuProfile::on_settings_changed));
}

sigc::signal<void, const Glib::ustring &> MenuProfile::signal_changed()
{
    return m_signal_changed;
}

double MenuProfile::get_opacity()
{
    return settings->get_double(STARTMENU_PROFILE_KEY_WINDOW_OPACITY);
}

MenuDisplayMode MenuProfile::get_display_mode()
{
    return static_cast<MenuDisplayMode>(settings->get_enum(STARTMENU_PROFILE_KEY_DISPLAY_MODE));
}

MenuDefaultPage MenuProfile::get_default_page()
{
    return static_cast<MenuDefaultPage>(settings->get_enum(STARTMENU_PROFILE_KEY_DEFAULT_PAGE));
}

void MenuProfile::set_opacity(double value)
{
    settings->set_double(STARTMENU_PROFILE_KEY_WINDOW_OPACITY, value);
}

void MenuProfile::set_default_page(MenuDefaultPage new_page)
{
    if (new_page >= PAGE_INVALID)
        return;
    settings->set_enum(STARTMENU_PROFILE_KEY_DEFAULT_PAGE, new_page);
}

void MenuProfile::set_display_mode(MenuDisplayMode new_mode)
{
    if (new_mode >= DISPLAY_MODE_INVALID)
        return;
    settings->set_enum(STARTMENU_PROFILE_KEY_DISPLAY_MODE, new_mode);
}

void MenuProfile::on_settings_changed(const Glib::ustring &key)
{
    KLOG_DEBUG("settings key '%s' changed\n", key.data());
    m_signal_changed.emit(key);
}
