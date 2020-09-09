#ifndef WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H
#define WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include <gtk/gtkx.h>
#include "kiran-window-thumbnail.h"
#include "kiranhelper.h"


class WorkspaceWindowThumbnail : public KiranWindowThumbnail
{
public:
    WorkspaceWindowThumbnail(KiranWindowPointer &window_, double scale_);
    ~WorkspaceWindowThumbnail();

    //窗口预览控件需要销毁
    sigc::signal<void> &signal_delete();
protected:
    virtual bool draw_snapshot(Gtk::Widget *area, const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;

    virtual void on_drag_data_delete (const Glib::RefPtr< Gdk::DragContext >& context) override;
    virtual void on_drag_data_get(const Glib::RefPtr< Gdk::DragContext >& context, Gtk::SelectionData& selection_data, guint info, guint time) override;
    virtual void on_drag_begin (const Glib::RefPtr< Gdk::DragContext >& context) override;

    virtual void on_close_button_clicked() override;
    virtual void on_thumbnail_clicked() override;

    virtual bool generate_thumbnail();

    void init_drag_and_drop();

private:
    double scale;
    int border_width;
    cairo_surface_t *thumbnail_surface;
    int thumbnail_width, thumbnail_height;
    sigc::signal<void> m_signal_delete;

    int window_width, window_height;
};

#endif // WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H
