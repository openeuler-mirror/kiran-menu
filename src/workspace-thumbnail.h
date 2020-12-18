#ifndef WORKSPACE_THUMBNAIL_INCLUDE_H
#define WORKSPACE_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include <auto_ptr.h>
#include "workspace.h"
#include "kiran-helper.h"
#include "kiran-thumbnail-widget.h"

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-bg.h>

class WorkspaceThumbnail : public KiranThumbnailWidget
{
public:
    WorkspaceThumbnail(KiranWorkspacePointer &workspace_);
    ~WorkspaceThumbnail() override;

    void set_current(bool current);
    bool get_is_current();
    sigc::signal<void, int> signal_selected();
    KiranWorkspacePointer get_workspace();

    void redraw_background();

protected:
    virtual bool on_button_press_event(GdkEventButton *event_button) override;
    virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) override;
    virtual bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) override;
    virtual void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection, guint info, guint time) override;

    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_close_button_clicked() override;
    virtual void on_thumbnail_clicked() override;

    bool reload_bg_surface();
    void init_drag_and_drop();
    
private:
    std::weak_ptr<Kiran::Workspace> workspace;
    MateBG *bg;
    cairo_surface_t *bg_surface;
    Glib::RefPtr<Gio::Settings> settings;
    int surface_width, surface_height;
    double surface_scale;

    bool is_current;
    int border_width;

    Gtk::Widget *thumbnail_area;

    bool drop_check;
    sigc::signal<void, int> m_signal_selected;
};

#endif
