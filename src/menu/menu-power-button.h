#ifndef MENU_POWER_BUTTON_H
#define MENU_POWER_BUTTON_H

#include <gtkmm.h>
#include "menu-power-menu.h"

class MenuPowerButton : public Gtk::Button
{
public:
    MenuPowerButton();
    ~MenuPowerButton();

protected:
    virtual void on_clicked() override;

private:
    Gtk::Image icon;
    Gtk::Menu *menu;
};

#endif // MENU_POWER_BUTTON_H
