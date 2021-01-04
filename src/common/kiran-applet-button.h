#ifndef KIRANAPPLETBUTTON_H
#define KIRANAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class KiranAppletButton : public Gtk::ToggleButton
{
public:
    KiranAppletButton(MatePanelApplet *applet_);
    ~KiranAppletButton() override;

    MatePanelApplet *get_applet();

protected:
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void set_icon_from_resource(const std::string &resource);
    void generate_pixbuf();

private:
    MatePanelApplet *applet;

    std::string icon_resource;
    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;      //图标
    int icon_size;                              //图标显示尺寸
};

#endif // KIRANAPPLETBUTTON_H
