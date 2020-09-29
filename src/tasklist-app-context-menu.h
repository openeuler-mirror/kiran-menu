#ifndef MENU_APP_CONTEXT_MENU_H
#define MENU_APP_CONTEXT_MENU_H

#include "kiran-opacity-menu.h"
#include "app.h"

class TasklistAppContextMenu : public KiranOpacityMenu
{
public:
    TasklistAppContextMenu(const std::shared_ptr<Kiran::App> &app);

    void refresh();

private:
    std::weak_ptr<Kiran::App> app;
};

#endif // MENU_APP_CONTEXT_MENU_H
