#ifndef WORKSPACE_APPLET_WINDOW_INCLUDE_H
#define WORKSPACE_APPLET_WINDOW_INCLUDE_H

#include <gtkmm.h>
#include "workspace-thumbnail.h"
#include "workspace-windows-overview.h"

class WorkspaceAppletWindow : public Gtk::Window
{
public:
    WorkspaceAppletWindow();

protected:
    virtual void on_realize() override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    void update_ui();

    virtual void on_map() override;
    void update_workspace(int workspace_num);

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *left_layout, *content_layout;
    Gtk::Box *right_layout;
    //std::vector<WorkspaceThumbnail*> ws_list;
    std::map<int, WorkspaceThumbnail*> ws_list;
    WorkspaceWindowsOverview overview;
    int current_workspace;
};

#endif // WORKSPACE_APPLET_WINDOW_INCLUDE_H
