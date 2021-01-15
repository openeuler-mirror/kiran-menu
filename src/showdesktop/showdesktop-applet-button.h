#ifndef SHOWDESKTOPAPPLETBUTTON_H
#define SHOWDESKTOPAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class ShowDesktopAppletButton : public Gtk::ToggleButton
{
public:
    ShowDesktopAppletButton(MatePanelApplet *applet_);

protected:
    virtual void on_map() override;
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_show_desktop_changed(bool show);
    virtual void on_toggled();

private:
    MatePanelApplet *applet;

    Gtk::Orientation get_applet_orientation() const;
};

#endif // SHOWDESKTOPAPPLETBUTTON_H