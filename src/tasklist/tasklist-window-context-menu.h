/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#ifndef TASKLIST_WINDOW_CONTEXT_MENU_H
#define TASKLIST_WINDOW_CONTEXT_MENU_H

#include "app-manager.h"
#include "kiran-opacity-menu.h"

class TasklistWindowContextMenu : public KiranOpacityMenu
{
public:
    explicit TasklistWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);

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
