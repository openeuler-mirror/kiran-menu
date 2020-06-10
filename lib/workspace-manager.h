/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 10:00:15
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace-manager.h
 */

#pragma once

#include "lib/workspace.h"

namespace Kiran
{
class WorkspaceManager
{
   public:
    virtual ~WorkspaceManager();

    static WorkspaceManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

    WorkspaceVec get_workspaces() { return this->workspaces_; }
    int32_t get_workspace_count() { return this->workspaces_.size(); }
    void change_workspace_count(int32_t count);

   private:
    void load_workspaces();

   private:
    static WorkspaceManager *instance_;

    WorkspaceVec workspaces_;
};
}  // namespace Kiran