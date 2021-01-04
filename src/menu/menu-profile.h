#ifndef KIRANMENUPROFILE_H
#define KIRANMENUPROFILE_H

#include <gtkmm.h>

typedef enum {
    DISPLAY_MODE_COMPACT=0,
    DISPLAY_MODE_EXPAND,
    DISPLAY_MODE_INVALID
} MenuDisplayMode;

typedef enum {
    PAGE_FAVORITES = 0,
    PAGE_ALL_APPS,
    PAGE_INVALID
} MenuDefaultPage;

class MenuProfile: public sigc::trackable
{
public:
    MenuProfile();
    sigc::signal<void, const Glib::ustring&> signal_changed();


    double get_opacity();
    MenuDisplayMode get_display_mode();
    MenuDefaultPage get_default_page();

    void set_opacity(double value);
    void set_default_page(MenuDefaultPage new_page);
    void set_display_mode(MenuDisplayMode new_mode);

    void on_settings_changed(const Glib::ustring &key);

private:
    Glib::RefPtr<Gio::Settings> settings;

    sigc::signal<void, const Glib::ustring&> m_signal_changed;
};

#endif // KIRANMENUPROFILE_H
