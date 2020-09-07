#ifndef KIRANBUTTONITEM_H
#define KIRANBUTTONITEM_H

#include <gtkmm.h>

class KiranMenuListItem : public Gtk::Button
{
public:
    KiranMenuListItem(int icon_size,
                      Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);

    KiranMenuListItem(const Glib::RefPtr<Gio::Icon> &gicon,
                      const std::string &text,
                      Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                      int icon_size=24);

    KiranMenuListItem(const std::string &resource,
                      const std::string &text,
                      Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                      int icon_size=24);

    void set_text(const std::string &text);
    void set_icon(const Glib::RefPtr<Gio::Icon> &gicon, int icon_size=24);
    void set_icon(const std::string &resource, int icon_size=24);
    void set_icon_size(int icon_size);
    virtual void set_orientation(Gtk::Orientation orient);

    void init_ui();

protected:
    virtual void on_style_updated() override;
    virtual bool on_draw(const::Cairo::RefPtr<Cairo::Context> &cr) override;

    void    on_orient_changed();

    virtual void get_preferred_width_vfunc(int &minimum_width,int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height,int &natural_height) const override;


private:
    Gtk::Label label;
    Gtk::Image image;
    Gtk::Box   box;

    Gtk::StyleProperty<int> space_property;
    Gtk::StyleProperty<int> max_width_property;
    Gtk::StyleProperty<int> max_height_property;
};

#endif // KIRANBUTTONITEM_H
