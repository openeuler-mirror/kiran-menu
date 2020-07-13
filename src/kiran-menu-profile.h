#ifndef KIRANMENUPROFILE_H
#define KIRANMENUPROFILE_H

#include <gtkmm.h>

typedef enum {
    DISPLAY_MODE_COMPACT=0,
    DISPLAY_MODE_EXPAND,
} MenuDisplayMode;

class KiranMenuProfile: public sigc::trackable
{
public:
    KiranMenuProfile();
    sigc::signal<void, const Glib::ustring&> signal_changed();


    Gdk::RGBA get_background_color();
    double get_opacity();
    MenuDisplayMode get_display_mode();

    void set_background_color(const Gdk::RGBA &color);
    void set_opacity(double value);

    void on_settings_changed(const Glib::ustring &key);

private:
    Glib::RefPtr<Gio::Settings> settings;

    sigc::signal<void, const Glib::ustring&> m_signal_changed;
};

#endif // KIRANMENUPROFILE_H
