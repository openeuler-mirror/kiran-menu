/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-15 11:11:34
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace-manager.h
 */

#pragma once

#include "lib/workspace.h"

namespace Kiran
{
// struct LessWorkspace
// {
//     bool operator()(const std::shared_ptr<Workspace> &x, const std::shared_ptr<Workspace> &y) const
//     {
//         return x->get_number() < y->get_number();
//     }
// };

class WorkspaceManager
{
   public:
    virtual ~WorkspaceManager();

    static WorkspaceManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

    // 获取所有的工作区
    WorkspaceVec get_workspaces();

    // 设置工作区的数量
    void change_workspace_count(int32_t count);

    // 通过索引获取工作区
    std::shared_ptr<Workspace> get_workspace(int32_t number);

    // 通过WnckWorkspace搜索Workspace对象
    std::shared_ptr<Workspace> lookup_workspace(WnckWorkspace *wnck_workspace);

    // 工作区创建信号
    sigc::signal<void, std::shared_ptr<Workspace>> &signal_workspace_created() { return this->workspace_created_; }
    // 工作区销毁信号
    sigc::signal<void, std::shared_ptr<Workspace>> &signal_workspace_destroyed() { return this->workspace_destroyed_; }

   private:
    WorkspaceManager();

    void load_workspaces();

    static void workspace_created(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data);
    static void workspace_destroyed(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data);

   private:
    static WorkspaceManager *instance_;

    sigc::signal<void, std::shared_ptr<Workspace>> workspace_created_;
    sigc::signal<void, std::shared_ptr<Workspace>> workspace_destroyed_;

    std::map<uint64_t, std::shared_ptr<Workspace>> workspaces_;
};
}  // namespace Kiran