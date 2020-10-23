#ifndef TASKLIST_WINDOW_PREVIEWER_H
#define TASKLIST_WINDOW_PREVIEWER_H

#include <gtkmm.h>
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
#include "window.h"
#include "tasklist-window-context-menu.h"
#include "window-thumbnail-widget.h"


#include <sigc++/sigc++.h>

class TasklistWindowPreviewer : public WindowThumbnailWidget
{
    friend GdkFilterReturn target_event_filter (GdkXEvent *xevent,
                      GdkEvent *event,
                      gpointer data);
public:
    TasklistWindowPreviewer(std::shared_ptr<Kiran::Window> &window);
    virtual ~TasklistWindowPreviewer() override;

    bool context_menu_is_opened();

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
    TasklistWindowContextMenu *context_menu;
};

#endif // TASKLIST_WINDOW_PREVIEWER_H
