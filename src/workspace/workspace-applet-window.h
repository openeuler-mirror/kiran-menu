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

#ifndef WORKSPACE_APPLET_WINDOW_INCLUDE_H
#define WORKSPACE_APPLET_WINDOW_INCLUDE_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "workspace-thumbnail.h"
#include "workspace-windows-overview.h"

// 工作区切换预览窗口 窗口左侧为工作区列表，右侧显示左侧所选工作区内所有窗口的缩略图
class WorkspaceAppletWindow : public Gtk::Window
{
public:
    /**
     * @brief 构造函数
     */
    WorkspaceAppletWindow();

protected:
    virtual void on_realize() override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_key_press_event(GdkEventKey *key) override;
    virtual void on_map() override;
    virtual bool on_map_event(GdkEventAny *event) override;
    virtual void on_unmap() override;

    /**
     * @brief 初始化窗口内的布局和其它界面元素
     */
    virtual void init_ui();

    /**
     * @brief 回调函数: 新的工作区创建后调用
     * @param workspace  新创建的工作区
     */
    virtual void on_workspace_created(std::shared_ptr<Kiran::Workspace> workspace);

    /**
     * @brief 回调函数: 工作区被删除后调用
     * @param workspace  被删除的工作区
     */
    virtual void on_workspace_destroyed(std::shared_ptr<Kiran::Workspace> workspace);

    /**
     * @brief 加载工作区列表
     */
    void load_workspaces();

    /**
     * @brief 更新指定的工作区的缩略图
     *        如果指定的工作区处于选中状态，窗口缩略图概览也会一起更新
     * @param workspace_num 要更新的工作区编号
     */
    void update_workspace(int workspace_num);

    /**
     * @brief 设置当前窗口在所有工作区上显示
     */
    void set_on_all_workspaces();

    /**
     * @brief 根据当前屏幕尺寸调整窗口的显示大小和位置
     */
    void resize_and_reposition();

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *main_layout; /* 窗口布局 */
    Gtk::Box *left_layout, *right_layout;
    WorkspaceWindowsOverview overview_area; /* 窗口缩略图显示区域 */

    MatePanelApplet *applet;                              /* 关联的面板插件 */
    std::map<int, WorkspaceThumbnail *> workspaces_table; /* 工作区编号到工作区缩略图控件的映射表 */
    int selected_workspace;                               /* 当前选择显示的工作区编号 */
};

#endif  // WORKSPACE_APPLET_WINDOW_INCLUDE_H
