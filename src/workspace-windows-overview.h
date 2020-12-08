#ifndef WORKSPACEWINDOWSOVERVIEW_H
#define WORKSPACEWINDOWSOVERVIEW_H

#include <gtkmm.h>
#include "workspace-manager.h"
#include "kiran-helper.h"

class WorkspaceWindowsOverview : public Gtk::EventBox
{
public:
    WorkspaceWindowsOverview();
    ~WorkspaceWindowsOverview();
    void set_workspace(KiranWorkspacePointer &workspace_);
    void reload();

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_button_press_event(GdkEventButton *event) override;


private:
    std::weak_ptr<Kiran::Workspace> workspace;
    Gtk::Box layout;

    int calculate_rows(std::vector<std::shared_ptr<Kiran::Window>> &windows,
                       int view_width,
                       int view_height,
                       int row_spacing,
                       int max_rows = 4);

    Gtk::Allocation old_allocation;
    sigc::connection reload_handler;
};

#endif // WORKSPACEWINDOWSOVERVIEW_H
