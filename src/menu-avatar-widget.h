#ifndef MENU_AVATAR_WIDGET_H
#define MENU_AVATAR_WIDGET_H

#include <gtkmm.h>

class MenuAvatarWidget : public Gtk::EventBox
{
public:
    MenuAvatarWidget(int size);
    void set_icon(const Glib::ustring &icon_file);
    void set_icon_size(int size_);

protected:
    virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr) override;

private:
    Glib::ustring icon;
    int icon_size;
};

#endif // MENU_AVATAR_WIDGET_H
