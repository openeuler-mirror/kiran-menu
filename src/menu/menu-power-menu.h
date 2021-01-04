#ifndef MENU_POWER_MENU_H
#define MENU_POWER_MENU_H

#include <gtkmm.h>
#include "kiran-opacity-menu.h"

class MenuPowerMenu : public KiranOpacityMenu
{
public:
    MenuPowerMenu();
    ~MenuPowerMenu() = default;

    void hide_menu_window();

private:
    Gtk::MenuItem *suspend_item;
    Gtk::MenuItem *hibernate_item;
    Gtk::MenuItem *shutdown_item;
    Gtk::MenuItem *reboot_item;
    Gtk::MenuItem *logout_item;
    Gtk::MenuItem *switchuser_item;
};

#endif // MENU_POWER_MENU_H
