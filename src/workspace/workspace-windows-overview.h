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

#ifndef WORKSPACEWINDOWSOVERVIEW_H
#define WORKSPACEWINDOWSOVERVIEW_H

#include <gtkmm.h>
#include "kiran-helper.h"
#include "workspace-manager.h"

// 工作区预览窗口中窗口缩略图显示区域控件
class WorkspaceWindowsOverview : public Gtk::EventBox
{
public:
    WorkspaceWindowsOverview();
    ~WorkspaceWindowsOverview();
    /**
     * @brief 显示给定的工作区workspace_中所有窗口的缩略图
     *        设置后，会自动触发刷新窗口缩略图
     * @param workspace_ 要显示缩略图的工作区
     */
    void set_workspace(KiranWorkspacePointer &workspace_);

    /**
     * @brief 设置缩略图之间的纵向间距
     * @param spacing 新的间距
     */
    void set_row_spacing(int spacing);

    /**
     * @brief 设置缩略图之间的横向间距
     * @param spacing 新的间距
     */
    void set_column_spacing(int spacing);

    /**
     * @brief 设置缩略图显示的最大行数
     * @param max_rows_ 最大行数
     */
    void set_max_rows(int max_rows_);

    /**
     * @brief 清除所有的窗口缩略图，不再显示
     */
    void clear();

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_button_press_event(GdkEventButton *event) override;

    /**
     * @brief 重新计算并排列当前工作区内的窗口缩略图
     */
    void reload_thumbnails();

    /**
     * @brief 计算并返回均匀显示给定窗口列表的缩略图所需的行数
     * @param windows       要显示缩略图的窗口列表
     * @param view_width    显示区域宽度
     * @param view_height   显示区域高度
     * @return 显示所需的行数
     */
    int calculate_rows(std::vector<std::shared_ptr<Kiran::Window>> &windows,
                       int view_width,
                       int view_height);

private:
    std::weak_ptr<Kiran::Workspace> workspace; /* 当前显示的工作区 */
    Gtk::Allocation old_allocation;            /* 缓存的显示区域大小 */
    sigc::connection reload_handler;           /* 缩略图加载处理器，用来避免重复加载 */

    Gtk::Box layout; /* 窗口布局 */

    int row_spacing, column_spacing; /* 缩略图之间横向间距和纵向间距 */
    int max_rows;                    /* 缩略图显示的最大行数 */
};

#endif  // WORKSPACEWINDOWSOVERVIEW_H
