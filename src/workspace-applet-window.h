#ifndef WORKSPACE_APPLET_WINDOW_INCLUDE_H
#define WORKSPACE_APPLET_WINDOW_INCLUDE_H

#include <gtkmm.h>
#include "workspace-thumbnail.h"
#include "workspace-windows-overview.h"
#include <mate-panel-applet.h>

class WorkspaceAppletWindow : public Gtk::Window
{
public:
    WorkspaceAppletWindow();

protected:
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_realize() override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_key_press_event(GdkEventKey *key) override;
    void update_ui();

    virtual void on_map() override;
    void update_workspace(int workspace_num);
    void set_on_all_workspaces();

private:
    MatePanelApplet *applet;
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *left_layout, *content_layout;
    Gtk::Box *right_layout;
    std::map<int, WorkspaceThumbnail*> ws_list;
    WorkspaceWindowsOverview overview;
    int selected_workspace;

};

#endif // WORKSPACE_APPLET_WINDOW_INCLUDE_H
