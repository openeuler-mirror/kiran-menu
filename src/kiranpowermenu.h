#ifndef KIRANPOWERMENU_H
#define KIRANPOWERMENU_H

#include <gtkmm.h>

class KiranPowerMenu : public Gtk::Menu
{
public:
    KiranPowerMenu();
    ~KiranPowerMenu();

    void hide_menu_window();

protected:
    virtual bool on_map_event(GdkEventAny *any_event) override;
    virtual bool on_focus_in_event(GdkEventFocus *focus_event) override;

private:
    Gtk::MenuItem suspend_item;
    Gtk::MenuItem hibernate_item;
    Gtk::MenuItem shutdown_item;
    Gtk::MenuItem reboot_item;
    Gtk::MenuItem logout_item;
};

#endif // KIRANPOWERMENU_H
