#ifndef KIRANOPACITYMENU_H
#define KIRANOPACITYMENU_H

#include <gtkmm.h>

class KiranOpacityMenu : public Gtk::Menu
{
public:
    KiranOpacityMenu();
    virtual ~KiranOpacityMenu() = default;

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
};

#endif // KIRANOPACITYMENU_H
