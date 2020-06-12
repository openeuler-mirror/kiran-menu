#ifndef KIRANMENUPROFILE_H
#define KIRANMENUPROFILE_H

#include <gtkmm.h>

class KiranMenuProfile: public sigc::trackable
{
public:
    KiranMenuProfile();
    sigc::signal<void> signal_changed();


    Gdk::RGBA get_background_color();
    double get_opacity();

    void set_background_color(const Gdk::RGBA &color);
    void set_opacity(double value);

    void on_settings_changed();

private:
    Glib::RefPtr<Gio::Settings> settings;

    sigc::signal<void> m_signal_changed;
};

#endif // KIRANMENUPROFILE_H
