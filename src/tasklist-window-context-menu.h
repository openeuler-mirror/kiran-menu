#ifndef TASKLIST_WINDOW_CONTEXT_MENU_H
#define TASKLIST_WINDOW_CONTEXT_MENU_H

#include "kiran-opacity-menu.h"
#include "app-manager.h"

class TasklistWindowContextMenu : public KiranOpacityMenu
{
public:
    TasklistWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);

    /**
     * @brief refresh 根据关联的窗口状态刷新菜单内容
     */
    void refresh();
protected:
    /**
     * @brief create_workspace_submenu 创建工作区列表子菜单，供工作区切换使用
     * @return 创建的子菜单
     */
    Gtk::Menu *create_workspace_submenu(void);

    /**
     * @brief move_window_to_workspace 将关联的窗口移动到指定的工作区
     * @param workspace_no             目标工作区的下标
     */
    void move_window_to_workspace(int workspace_no);

private:
    std::weak_ptr<Kiran::Window> win;   /* 该菜单关联的窗口 */
};

#endif // TASKLIST_WINDOW_CONTEXT_MENU_H
