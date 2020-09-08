/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 13:55:27
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace-manager.cpp
 */

#include "lib/workspace-manager.h"

#include "lib/helper.h"

namespace Kiran
{
WorkspaceManager::WorkspaceManager(WindowManager *window_manager) : window_manager_(window_manager),
                                                                    created_handler_(0),
                                                                    destroyed_handler_(0),
                                                                    active_changed_handler_(0)
{
}

WorkspaceManager::~WorkspaceManager()
{
    auto screen = wnck_screen_get_default();
    if (screen)
    {
        if (this->created_handler_)
        {
            g_signal_handler_disconnect(screen, this->created_handler_);
        }
        if (this->destroyed_handler_)
        {
            g_signal_handler_disconnect(screen, this->destroyed_handler_);
        }
        if (this->active_changed_handler_)
        {
            g_signal_handler_disconnect(screen, this->active_changed_handler_);
        }
    }
}

WorkspaceManager *WorkspaceManager::instance_ = nullptr;
void WorkspaceManager::global_init(WindowManager *window_manager)
{
    instance_ = new WorkspaceManager(window_manager);
    instance_->init();
}

void WorkspaceManager::init()
{
    load_workspaces();

    this->window_manager_->signal_window_opened().connect(sigc::mem_fun(this, &WorkspaceManager::window_opened));
    this->window_manager_->signal_window_closed().connect(sigc::mem_fun(this, &WorkspaceManager::window_closed));

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    this->created_handler_ = g_signal_connect(screen, "workspace-created", G_CALLBACK(WorkspaceManager::workspace_created), this);
    this->destroyed_handler_ = g_signal_connect(screen, "workspace-destroyed", G_CALLBACK(WorkspaceManager::workspace_destroyed), this);
    this->active_changed_handler_ = g_signal_connect(screen, "active-workspace-changed", G_CALLBACK(WorkspaceManager::active_workspace_changed), this);
}

WorkspaceVec WorkspaceManager::get_workspaces()
{
    WorkspaceVec workspaces;

    for (auto iter = this->workspaces_.begin(); iter != this->workspaces_.end(); ++iter)
    {
        if (iter->second->get_number() != workspaces.size())
        {
            g_warning("the number of the workspace is invalid. number: %d name: %s need_number: %d\n",
                      iter->second->get_number(),
                      iter->second->get_name().c_str(),
                      workspaces.size());
        }

        workspaces.push_back(iter->second);
    }
    return workspaces;
}

std::shared_ptr<Workspace> WorkspaceManager::get_active_workspace()
{
    auto screen = wnck_screen_get_default();
    g_return_val_if_fail(screen != NULL, nullptr);

    auto wnck_workspace = wnck_screen_get_active_workspace(screen);

    return lookup_workspace(wnck_workspace);
}

void WorkspaceManager::change_workspace_count(int32_t count)
{
    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    wnck_screen_change_workspace_count(screen, count);
}

std::shared_ptr<Workspace> WorkspaceManager::get_workspace(int32_t number)
{
    RETURN_VAL_IF_TRUE(number < 0, nullptr);

    auto iter = this->workspaces_.find(number);
    if (iter != this->workspaces_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<Workspace> WorkspaceManager::lookup_workspace(WnckWorkspace *wnck_workspace)
{
    RETURN_VAL_IF_FALSE(wnck_workspace != NULL, nullptr);

    auto number = wnck_workspace_get_number(wnck_workspace);
    auto iter = this->workspaces_.find(number);
    if (iter == this->workspaces_.end())
    {
        g_warning("not found the workspace. number: %d name: %s\n",
                  number,
                  wnck_workspace_get_name(wnck_workspace));
        return nullptr;
    }
    else
    {
        return iter->second;
    }
}

void WorkspaceManager::load_workspaces()
{
    this->workspaces_.clear();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    auto wnck_workspaces = wnck_screen_get_workspaces(screen);

    for (GList *l = wnck_workspaces; l != NULL; l = l->next)
    {
        auto wnck_workspace = (WnckWorkspace *)(l->data);
        auto workspace = std::make_shared<Workspace>(wnck_workspace);
        auto id = workspace->get_number();
        this->workspaces_[id] = workspace;
    }
}

void WorkspaceManager::window_opened(std::shared_ptr<Window> window)
{
    if (window->is_pinned())
    {
        for (const auto &workspace : this->workspaces_)
        {
            workspace.second->add_window(window);
        }
    }
    else
    {
        auto workspace = window->get_workspace();
        if (workspace)
        {
            workspace->add_window(window);
        }
    }
}

void WorkspaceManager::window_closed(std::shared_ptr<Window> window)
{
    if (window->is_pinned())
    {
        for (const auto &workspace : this->workspaces_)
        {
            workspace.second->remove_window(window);
        }
    }
    else
    {
        auto workspace = window->get_workspace();
        if (workspace)
        {
            workspace->remove_window(window);
        }
    }
}

void WorkspaceManager::workspace_created(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data)
{
    auto workspace_manager = (WorkspaceManager *)user_data;

    g_return_if_fail(wnck_workspace != NULL);
    g_return_if_fail(workspace_manager == WorkspaceManager::get_instance());

    auto number = wnck_workspace_get_number(wnck_workspace);
    auto workspace = std::make_shared<Workspace>(wnck_workspace);

    auto iter = workspace_manager->workspaces_.find(number);
    if (iter == workspace_manager->workspaces_.end())
    {
        workspace_manager->workspaces_[number] = workspace;
    }
    else
    {
        g_warning("the workspace is already exist. number: %d old_name: %s new_name: %s\n",
                  number,
                  iter->second->get_name().c_str(),
                  workspace->get_name().c_str());
        iter->second = workspace;
    }

    // 添加pin窗口到工作区
    for (const auto &window : WindowManager::get_instance()->get_windows())
    {
        if (window->is_pinned())
        {
            workspace->add_window(window);
        }
    }
    workspace_manager->workspace_created_.emit(workspace);
}

void WorkspaceManager::workspace_destroyed(WnckScreen *screen, WnckWorkspace *wnck_workspace, gpointer user_data)
{
    auto workspace_manager = (WorkspaceManager *)user_data;

    g_return_if_fail(wnck_workspace != NULL);
    g_return_if_fail(workspace_manager == WorkspaceManager::get_instance());

    auto number = wnck_workspace_get_number(wnck_workspace);

    auto iter = workspace_manager->workspaces_.find(number);
    if (iter == workspace_manager->workspaces_.end())
    {
        g_warning("not found the workspace. number: %d name: %s\n",
                  number,
                  wnck_workspace_get_name(wnck_workspace));
    }
    else
    {
        workspace_manager->workspace_destroyed_.emit(iter->second);
        workspace_manager->workspaces_.erase(iter);
    }
}

void WorkspaceManager::active_workspace_changed(WnckScreen *screen, WnckWorkspace *prev_wnck_workspace, gpointer user_data)
{
    auto workspace_manager = (WorkspaceManager *)user_data;
    g_return_if_fail(workspace_manager == WorkspaceManager::get_instance());

    auto prev_workspace = workspace_manager->lookup_workspace(prev_wnck_workspace);
    auto cur_workspace = workspace_manager->get_active_workspace();

    workspace_manager->active_workspace_changed_.emit(prev_workspace, cur_workspace);
}

}  // namespace Kiran
