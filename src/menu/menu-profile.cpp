/**
 * @file          /kiran-menu/src/menu/menu-profile.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
