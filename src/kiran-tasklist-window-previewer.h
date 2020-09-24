#ifndef KIRANWINDOWPREVIEWER_H
#define KIRANWINDOWPREVIEWER_H

#include <gtkmm.h>
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
#include "window.h"
#include "kiran-window-context-menu.h"
#include "kiran-window-thumbnail.h"


#include <sigc++/sigc++.h>

class KiranWindowPreviewer : public KiranWindowThumbnail
{
    friend GdkFilterReturn target_event_filter (GdkXEvent *xevent,
                      GdkEvent *event,
                      gpointer data);
public:
    KiranWindowPreviewer(std::shared_ptr<Kiran::Window> &window);
    virtual ~KiranWindowPreviewer() override;

    sigc::signal<void> signal_close();

protected:
    virtual void get_preferred_width_vfunc(int& minimum_width,
                                           int& natural_width) const override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) override;

    virtual void on_thumbnail_clicked() override;
    virtual void on_close_button_clicked() override;

    virtual bool on_button_press_event(GdkEventButton *event) override;
    void on_composite_changed();

private:
    KiranWindowContextMenu *context_menu;

    sigc::signal<void> m_signal_close;
};

#endif // KIRANWINDOWPREVIEWER_H
