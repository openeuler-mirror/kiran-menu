#ifndef KIRANMENUPOWERMENU_H
#define KIRANMENUPOWERMENU_H

#include <gtkmm.h>

class KiranMenuPowerMenu : public Gtk::Menu
{
public:
    KiranMenuPowerMenu();
    ~KiranMenuPowerMenu();

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

#endif // KIRANMENUPOWERMENU_H
