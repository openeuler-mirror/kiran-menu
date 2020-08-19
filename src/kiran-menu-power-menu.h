#ifndef KIRANMENUPOWERMENU_H
#define KIRANMENUPOWERMENU_H

#include <gtkmm.h>
#include "kiran-opacity-menu.h"

class KiranMenuPowerMenu : public KiranOpacityMenu
{
public:
    KiranMenuPowerMenu();
    ~KiranMenuPowerMenu() = default;

    void hide_menu_window();

private:
    Gtk::MenuItem *suspend_item;
    Gtk::MenuItem *hibernate_item;
    Gtk::MenuItem *shutdown_item;
    Gtk::MenuItem *reboot_item;
    Gtk::MenuItem *logout_item;
    Gtk::MenuItem *switchuser_item;
};

#endif // KIRANMENUPOWERMENU_H
