/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 12:01:05
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace-manager.h
 */

#pragma once

#include "lib/window-manager.h"
#include "lib/workspace.h"

namespace Kiran
{
class WorkspaceManager
{
public:
    virtual ~WorkspaceManager();

    static WorkspaceManager *get_instance() { return instance_; };

    static void global_init(WindowManager *window_manager);

    static void global_deinit() { delete instance_; };

    void init();

    // 获取所有的工作区
    WorkspaceVec get_workspaces();

    // 获取当前活动的工作区
    std::shared_ptr<Workspace> get_active_workspace();

    // 设置工作区的数量
    void change_workspace_count(int32_t count);

    // 通过索引获取工作区
    std::shared_ptr<Workspace> get_workspace(int32_t number);

    /**
     * @brief 移除指定的工作区
     *        由于工作区只支持数量设置，我们只能移除最后一个工作区。
     *        所以在移除指定的工作区时，需要逐个将要移除的工作区以及之后的工作区中的所有窗口移动到
     *        上一个工作区，然后再移除最后的工作区。
     *        如果workspace参数为nullptr，什么都不做.
     * @param workspace  要移除的工作区
     */
    void destroy_workspace(std::shared_ptr<Workspace> workspace);

    // 通过WnckWorkspace搜索Workspace对象
    std::shared_ptr<Workspace> lookup_workspace(WnckWorkspace *wnck_workspace);

    // 工作区创建信号
    sigc::signal<void, std::shared_ptr<Workspace>> &signal_workspace_created() { return this->workspace_created_; }
    // 工作区销毁信号
    sigc::signal<void, std::shared_ptr<Workspace>> &signal_workspace_destroyed() { return this->workspace_destroyed_; }
    // 活动的工作区发生变化，参数分别为：返回值，上一次活动的工作区，当前活动的工作区
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> &signal_active_workspace_changed() { return this->active_workspace_changed_; }

private:
    WorkspaceManager(WindowManager *window_manager);

    void load_workspaces();

    // 处理窗口被打开的信号
    void window_opened(std::shared_ptr<Window> window);
    // 处理窗口关闭的信号
    void window_closed(std::shared_ptr<Window> window);
    // 处理工作区被创建的信号
    static void workspace_created(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data);
    // 处理工作区被销毁的信号
    static void workspace_destroyed(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data);
    // 处理活动的工作区变化的信号
    static void active_workspace_changed(WnckScreen *screen, WnckWorkspace *prev_wnck_workspace, gpointer user_data);

private:
    static WorkspaceManager *instance_;

    WindowManager *window_manager_;

    sigc::signal<void, std::shared_ptr<Workspace>> workspace_created_;
    sigc::signal<void, std::shared_ptr<Workspace>> workspace_destroyed_;
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> active_workspace_changed_;

    uint64_t created_handler_;
    uint64_t destroyed_handler_;
    uint64_t active_changed_handler_;

    std::map<uint64_t, std::shared_ptr<Workspace>> workspaces_;
};
}  // namespace Kiran