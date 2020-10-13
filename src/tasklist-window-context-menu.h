#ifndef TASKLIST_WINDOW_CONTEXT_MENU_H
#define TASKLIST_WINDOW_CONTEXT_MENU_H

#include "kiran-opacity-menu.h"
#include "app-manager.h"

class TasklistWindowContextMenu : public KiranOpacityMenu
{
public:
    TasklistWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);
    void refresh();
protected:
    Gtk::Menu *create_workspace_menu(void);

private:
    std::weak_ptr<Kiran::Window> win;
};

#endif // TASKLIST_WINDOW_CONTEXT_MENU_H
