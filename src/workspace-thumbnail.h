#ifndef WORKSPACE_THUMBNAIL_INCLUDE_H
#define WORKSPACE_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include <auto_ptr.h>
#include "workspace.h"
#include "kiran-helper.h"

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-bg.h>

class WorkspaceThumbnail : public Gtk::Box
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

    virtual bool draw_snapshot(const Cairo::RefPtr<Cairo::Context> &cr);

    bool reload_bg_surface();
    void init_drag_and_drop();
    
private:
    std::weak_ptr<Kiran::Workspace> workspace;
    MateBG *bg;
    cairo_surface_t *bg_surface;
    Glib::RefPtr<Gio::Settings> settings;
    int surface_width, surface_height;
    double surface_scale;

    Gtk::Label name_label;
    Gtk::Button snapshot_area;
    bool is_current;
    int border_width;

    bool drop_check;
    sigc::signal<void, int> m_signal_selected;
};

#endif
