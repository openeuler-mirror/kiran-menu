#include "kiran-menu-profile.h"

KiranMenuProfile::KiranMenuProfile()
{
    settings = Gio::Settings::create("com.unikylin.Kiran.StartMenu.profile");
    settings->signal_changed().connect(
               sigc::mem_fun(*this, &KiranMenuProfile::on_settings_changed));
}

sigc::signal<void, const Glib::ustring &> KiranMenuProfile::signal_changed()
{
    return m_signal_changed;
}

Gdk::RGBA KiranMenuProfile::get_background_color()
{
    std::string color_setting;

    //从gsettings中读取颜色和透明度设置
    color_setting = settings->get_string("background-color");

    return Gdk::RGBA(color_setting);
}

double KiranMenuProfile::get_opacity()
{
    return settings->get_double("background-opacity");
}

MenuDisplayMode KiranMenuProfile::get_display_mode()
{
    return static_cast<MenuDisplayMode>(settings->get_enum("display-mode"));
}

void KiranMenuProfile::set_background_color(const Gdk::RGBA &color)
{
    settings->set_string("background-color", color.to_string());
}

void KiranMenuProfile::set_opacity(double value)
{
    settings->set_double("background-opacity", value);
}

void KiranMenuProfile::on_settings_changed(const Glib::ustring &key)
{
    g_message("settings key '%s' changed\n", key.data());
    m_signal_changed.emit(key);
}
