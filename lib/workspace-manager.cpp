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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "lib/workspace-manager.h"

#include "lib/base.h"

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
        if (iter->second->get_number() != (int)workspaces.size())
        {
            KLOG_WARNING("the number of the workspace is invalid. number: %d name: %s need_number: %d\n",
                         iter->second->get_number(),
                         iter->second->get_name().c_str(),
                         (int)workspaces.size());
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
    KLOG_PROFILE("count: %d.", count);

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

void WorkspaceManager::destroy_workspace(std::shared_ptr<Workspace> workspace)
{
    int workspace_no;
    std::shared_ptr<Workspace> prev_workspace, current_workspace;
    auto workspaces_list = get_workspaces();

    g_return_if_fail(workspace != nullptr);

    workspace_no = workspace->get_number();
    for (int i = workspace_no - 1;; i++)
    {
        if (i < 0)
            continue;

        prev_workspace = get_workspace(i);
        current_workspace = get_workspace(i + 1);
        if (prev_workspace == nullptr || current_workspace == nullptr)
        {
            break;
        }

        for (auto window : current_workspace->get_windows())
        {
            /* 过滤掉Dock窗口、Desktop窗口和在所有工作区中显示的窗口 */
            if (window->get_window_type() == WNCK_WINDOW_DOCK || window->get_window_type() == WNCK_WINDOW_DESKTOP)
                continue;

            if (window->is_pinned())
                continue;

            window->move_to_workspace(prev_workspace);
        }
    }

    /* 调整工作区数量，移除最后一个工作区 */
    change_workspace_count(workspaces_list.size() - 1);
}

std::shared_ptr<Workspace> WorkspaceManager::lookup_workspace(WnckWorkspace *wnck_workspace)
{
    RETURN_VAL_IF_FALSE(wnck_workspace != NULL, nullptr);

    auto number = wnck_workspace_get_number(wnck_workspace);
    auto iter = this->workspaces_.find(number);
    if (iter == this->workspaces_.end())
    {
        KLOG_WARNING("not found the workspace. number: %d name: %s\n",
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
    KLOG_PROFILE("");

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
    g_return_if_fail(window);
    KLOG_DEBUG("xid: %" PRIu64 ", name: %s.", window->get_xid(), window->get_name().c_str());

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
    g_return_if_fail(window);
    KLOG_DEBUG("xid: %" PRIu64 ", name: %s.", window->get_xid(), window->get_name().c_str());

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
    auto name = wnck_workspace_get_name(wnck_workspace);
    auto workspace = std::make_shared<Workspace>(wnck_workspace);

    KLOG_DEBUG("workspace is created. number: %d, name: %s.", number, name);

    auto iter = workspace_manager->workspaces_.find(number);
    if (iter == workspace_manager->workspaces_.end())
    {
        workspace_manager->workspaces_[number] = workspace;
    }
    else
    {
        KLOG_WARNING("the workspace is already exist. number: %d old_name: %s new_name: %s\n",
                     number,
                     iter->second->get_name().c_str(),
                     name);
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
    auto name = wnck_workspace_get_name(wnck_workspace);

    KLOG_DEBUG("workspace is destroyed. number: %d, name: %s.", number, name);

    auto iter = workspace_manager->workspaces_.find(number);
    if (iter == workspace_manager->workspaces_.end())
    {
        KLOG_WARNING("not found the workspace. number: %d name: %s\n", number, name);
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

    KLOG_DEBUG("prev workspace: number %d, name %s; cur workspace: number %d, name %s.",
               prev_workspace ? prev_workspace->get_number() : 0,
               prev_workspace ? prev_workspace->get_name().c_str() : "null",
               cur_workspace ? cur_workspace->get_number() : 0,
               cur_workspace ? cur_workspace->get_name().c_str() : "null");

    workspace_manager->active_workspace_changed_.emit(prev_workspace, cur_workspace);
}

}  // namespace Kiran
