#ifndef KIRANPOWERBUTTON_H
#define KIRANPOWERBUTTON_H

#include <gtkmm.h>
#include "kiranpowermenu.h"

class KiranPowerButton : public Gtk::Button
{
public:
    KiranPowerButton();
    ~KiranPowerButton();

protected:
    virtual void on_clicked() override;

private:
    Gtk::Image icon;
    Gtk::Menu *menu;
};

#endif // KIRANPOWERBUTTON_H
