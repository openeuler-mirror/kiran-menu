/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-09 17:05:32
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace-manager.cpp
 */

#include "lib/workspace-manager.h"

namespace Kiran
{
WorkspaceManager* WorkspaceManager::instance_ = nullptr;
void WorkspaceManager::global_init()
{
    instance_ = new WorkspaceManager();
    instance_->init();
}

void WorkspaceManager::init()
{
    load_workspaces();
}

void WorkspaceManager::change_workspace_count(int32_t count)
{
    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    wnck_screen_change_workspace_count(screen, count);
}

void WorkspaceManager::load_workspaces()
{
    this->workspaces_.clear();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    auto wnck_workspaces = wnck_screen_get_workspaces(screen);

    for (GList* l = wnck_workspaces; l != NULL; l = l->next)
    {
        auto wnck_workspace = (WnckWorkspace*)(l->data);
        auto workspace = std::make_shared<Workspace>(wnck_workspace);
        this->workspaces_.push_back(workspace);
    }
}

}  // namespace Kiran
