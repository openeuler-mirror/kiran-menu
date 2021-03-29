#ifndef MENU_POWER_BUTTON_H
#define MENU_POWER_BUTTON_H

#include <gtkmm.h>
#include "menu-power-menu.h"

class MenuPowerButton : public Gtk::Button
{
public:
    MenuPowerButton();
    ~MenuPowerButton();

    sigc::signal<void> signal_power_menu_deactivated();

protected:
    virtual void on_clicked() override;
    virtual void on_power_menu_deactivated();

private:
    Gtk::Image icon;
    Gtk::Menu *menu;

    sigc::signal<void> m_signal_power_menu_deactivated;
};

#endif // MENU_POWER_BUTTON_H
