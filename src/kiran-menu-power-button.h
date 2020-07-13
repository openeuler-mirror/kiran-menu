#ifndef KIRANMENUPOWERBUTTON_H
#define KIRANMENUPOWERBUTTON_H

#include <gtkmm.h>
#include "kiran-menu-power-menu.h"

class KiranMenuPowerButton : public Gtk::Button
{
public:
    KiranMenuPowerButton();
    ~KiranMenuPowerButton();

protected:
    virtual void on_clicked() override;

private:
    Gtk::Image icon;
    Gtk::Menu *menu;
};

#endif // KIRANMENUPOWERBUTTON_H
