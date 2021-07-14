/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

#include "menu-profile.h"
#include "lib/base.h"

const Glib::ustring default_page_key = "default-page";
const Glib::ustring display_mode_key = "display-mode";
const Glib::ustring window_opacity_key = "background-opacity";
const Glib::ustring profile_settings_path = "com.kylinsec.kiran.startmenu.profile";

MenuProfile::MenuProfile()
{
    settings = Gio::Settings::create(profile_settings_path);
    settings->signal_changed().connect(
        sigc::mem_fun(*this, &MenuProfile::on_settings_changed));
}

sigc::signal<void, const Glib::ustring &> MenuProfile::signal_changed()
{
    return m_signal_changed;
}

double MenuProfile::get_opacity()
{
    return settings->get_double(window_opacity_key);
}

MenuDisplayMode MenuProfile::get_display_mode()
{
    return static_cast<MenuDisplayMode>(settings->get_enum(display_mode_key));
}

MenuDefaultPage MenuProfile::get_default_page()
{
    return static_cast<MenuDefaultPage>(settings->get_enum(default_page_key));
}

void MenuProfile::set_opacity(double value)
{
    settings->set_double(window_opacity_key, value);
}

void MenuProfile::set_default_page(MenuDefaultPage new_page)
{
    if (new_page >= PAGE_INVALID)
        return;
    settings->set_enum(default_page_key, new_page);
}

void MenuProfile::set_display_mode(MenuDisplayMode new_mode)
{
    if (new_mode >= DISPLAY_MODE_INVALID)
        return;
    settings->set_enum(display_mode_key, new_mode);
}

void MenuProfile::on_settings_changed(const Glib::ustring &key)
{
    KLOG_DEBUG("settings key '%s' changed\n", key.data());
    m_signal_changed.emit(key);
}
