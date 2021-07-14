/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#ifndef TASKLIST_WINDOW_CONTEXT_MENU_H
#define TASKLIST_WINDOW_CONTEXT_MENU_H

#include "app-manager.h"
#include "kiran-opacity-menu.h"

class TasklistWindowContextMenu : public KiranOpacityMenu
{
public:
    TasklistWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);

    sigc::signal<void> signal_window_move_required();

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
    std::weak_ptr<Kiran::Window> win; /* 该菜单关联的窗口 */
    sigc::signal<void> m_signal_window_move_required;
};

#endif  // TASKLIST_WINDOW_CONTEXT_MENU_H
