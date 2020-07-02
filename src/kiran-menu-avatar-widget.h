#ifndef KIRANMENUAVATARWIDGET_H
#define KIRANMENUAVATARWIDGET_H

#include <gtkmm.h>

class KiranMenuAvatarWidget : public Gtk::EventBox
{
public:
    KiranMenuAvatarWidget(int size);
    void set_icon(const char *icon);

protected:
    virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr) override;
    virtual bool on_button_press_event (GdkEventButton* button_event) override;

private:
    std::string icon;
    int icon_size;
};

#endif // KIRANMENUAVATARWIDGET_H
