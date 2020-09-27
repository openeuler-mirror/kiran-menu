#include "kiran-menu-profile.h"

const Glib::ustring default_page_key = "default-page";
const Glib::ustring display_mode_key = "display-mode";
const Glib::ustring window_opacity_key = "background-opacity";
const Glib::ustring profile_settings_path = "com.unikylin.Kiran.StartMenu.profile";

KiranMenuProfile::KiranMenuProfile()
{
    settings = Gio::Settings::create(profile_settings_path);
    settings->signal_changed().connect(
               sigc::mem_fun(*this, &KiranMenuProfile::on_settings_changed));
}

sigc::signal<void, const Glib::ustring &> KiranMenuProfile::signal_changed()
{
    return m_signal_changed;
}

double KiranMenuProfile::get_opacity()
{
    return settings->get_double(window_opacity_key);
}

MenuDisplayMode KiranMenuProfile::get_display_mode()
{
    return static_cast<MenuDisplayMode>(settings->get_enum(display_mode_key));
}

MenuDefaultPage KiranMenuProfile::get_default_page()
{
    return static_cast<MenuDefaultPage>(settings->get_enum(default_page_key));
}

void KiranMenuProfile::set_opacity(double value)
{
    settings->set_double(window_opacity_key, value);
}

void KiranMenuProfile::set_default_page(MenuDefaultPage new_page)
{
    if (new_page >= PAGE_INVALID)
        return;
    settings->set_enum(default_page_key, new_page);
}

void KiranMenuProfile::set_display_mode(MenuDisplayMode new_mode)
{
    if (new_mode >= DISPLAY_MODE_INVALID)
        return;
    settings->set_enum(display_mode_key, new_mode);
}

void KiranMenuProfile::on_settings_changed(const Glib::ustring &key)
{
    g_message("settings key '%s' changed\n", key.data());
    m_signal_changed.emit(key);
}
