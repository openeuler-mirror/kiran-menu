#ifndef WORKSPACEWINDOWSOVERVIEW_H
#define WORKSPACEWINDOWSOVERVIEW_H

#include <gtkmm.h>
#include "workspace-manager.h"
#include "kiranhelper.h"

class WorkspaceWindowsOverview : public Gtk::EventBox
{
public:
    WorkspaceWindowsOverview();
    void set_workspace(KiranWorkspacePointer &workspace_);
    void reload();

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;


private:
    std::weak_ptr<Kiran::Workspace> workspace;
    Gtk::Box layout;

    int calculate_rows(std::vector<std::shared_ptr<Kiran::Window>> &windows,
                       int view_width,
                       int view_height,
                       int row_spacing,
                       int max_rows = 4);
};

#endif // WORKSPACEWINDOWSOVERVIEW_H
