#ifndef MENU_AVATAR_WIDGET_H
#define MENU_AVATAR_WIDGET_H

#include <gtkmm.h>

class MenuAvatarWidget : public Gtk::EventBox
{
public:
    MenuAvatarWidget(int size);
    void set_icon(const char *icon);

protected:
    virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr) override;
    virtual bool on_button_press_event (GdkEventButton* button_event) override;

private:
    std::string icon;
    int icon_size;
};

#endif // MENU_AVATAR_WIDGET_H
