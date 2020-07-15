#ifndef KIRANWINDOWPREVIEWER_H
#define KIRANWINDOWPREVIEWER_H

#include <gtkmm.h>
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
#include "window.h"
#include "kiran-window-context-menu.h"


#include <sigc++/sigc++.h>

class KiranWindowPreviewer : public Gtk::EventBox
{
    friend GdkFilterReturn target_event_filter (GdkXEvent *xevent,
                      GdkEvent *event,
                      gpointer data);
public:
    KiranWindowPreviewer(const std::shared_ptr<Kiran::Window> &window);
    virtual ~KiranWindowPreviewer() override;

    bool draw_snapshot(const Cairo::RefPtr<Cairo::Context> &cr);
    sigc::signal<void> signal_close();

protected:
    virtual Gtk::SizeRequestMode get_request_mode_vfunc() const override;
    virtual void get_preferred_width_vfunc(int& minimum_width,
                                           int& natural_width) const override;
    virtual void get_preferred_height_vfunc(int& minimum_width,
                                           int& natural_width) const override;


    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_button_press_event(GdkEventButton *button_event) override;

    void on_composite_changed();
    void init_ui();


private:
    std::weak_ptr<Kiran::Window> window;

    Gtk::Box layout, top_layout;
    Gtk::Image icon_image;
    Gtk::Label title_label;
    Gtk::Button close_btn;
    Gtk::DrawingArea snapshot_area;
    KiranWindowContextMenu *context_menu;

    sigc::signal<void> m_signal_close;
};

#endif // KIRANWINDOWPREVIEWER_H
