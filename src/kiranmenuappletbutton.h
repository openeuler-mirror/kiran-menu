#ifndef KIRANMENUAPPLETBUTTON_H
#define KIRANMENUAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiranmenuwindow.h"

class KiranMenuAppletButton : public Gtk::ToggleButton
{
public:
    KiranMenuAppletButton(MatePanelApplet *panel_applet);
    ~KiranMenuAppletButton();

protected:
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_toggled();

    void    update_icon();

private:
    MatePanelApplet *applet;
    KiranMenuWindow window;                     //菜单窗口

    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;      //图标
    int icon_size;                              //图标显示尺寸
    sigc::connection connection1, connection2;
};

#endif // KIRANMENUAPPLETBUTTON_H
