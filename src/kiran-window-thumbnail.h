#ifndef KIRAN_WINDOW_THUMBNAIL_INCLUDE_H
#define KIRAN_WINDOW_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include <window.h>
#include "kiranhelper.h"

class KiranWindowThumbnail: public Gtk::Button
{
public:
    KiranWindowThumbnail(KiranWindowPointer &window_);
    KiranWindowPointer get_window_() const;

protected:
    virtual bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    virtual void on_clicked() override;

    virtual void on_close_button_clicked();
    virtual void on_thumbnail_clicked();

    virtual bool draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr);
    virtual bool draw_close_button(Gtk::Widget *close_area, const Cairo::RefPtr<Cairo::Context> &cr);
    virtual bool draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr);

    virtual void init_ui();
    virtual void update_title();

    virtual void set_snapshot_size(int width, int height);

private:
    std::weak_ptr<Kiran::Window> window;
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Grid *layout;
    Gtk::Label *title_label;
    Gtk::DrawingArea *icon_area;
    Gtk::DrawingArea *snapshot_area, *close_area;
};


#endif
