/**
 * @file workspace-windows-overview.h
 * @brief 工作区预览窗口中窗口缩略图显示区域控件
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
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
    /**
     * @brief 显示给定的工作区workspace_中所有窗口的缩略图
     *        设置后，会自动触发刷新窗口缩略图
     * @param workspace_ 要显示缩略图的工作区
     */
    void set_workspace(KiranWorkspacePointer &workspace_);

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
     * @param row_spacing   缩略图显示的行间距
     * @param max_rows      缩略图显示的最大行数
     * @return 显示所需的行数
     */
    int calculate_rows(std::vector<std::shared_ptr<Kiran::Window>> &windows,
                       int view_width,
                       int view_height,
                       int row_spacing,
                       int max_rows = 4);

private:
    std::weak_ptr<Kiran::Workspace> workspace;      /* 当前显示的工作区 */
    Gtk::Allocation old_allocation;                 /* 缓存的显示区域大小 */
    sigc::connection reload_handler;                /* 缩略图加载处理器，用来避免重复加载 */

    Gtk::Box layout;                                /* 窗口布局 */
};

#endif // WORKSPACEWINDOWSOVERVIEW_H
