#ifndef MENU_APP_CONTEXT_MENU_H
#define MENU_APP_CONTEXT_MENU_H

#include "kiran-opacity-menu.h"
#include "app.h"

class TasklistAppContextMenu : public KiranOpacityMenu
{
public:
    /**
     * @brief TasklistAppContextMenu
     * @param app  右键菜单关联的应用
     */
    TasklistAppContextMenu(const std::shared_ptr<Kiran::App> &app);

    /**
     * @brief refresh 更新右键菜单内容
     */
    void refresh();

private:
    std::weak_ptr<Kiran::App> app;
};

#endif // MENU_APP_CONTEXT_MENU_H
