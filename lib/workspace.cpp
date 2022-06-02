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

#include "lib/workspace.h"

#include "lib/base.h"
#include "window-manager.h"
namespace Kiran
{
Workspace::Workspace(WnckWorkspace *workspace) : workspace_(workspace)
{
}

Workspace::~Workspace()
{
}

int Workspace::get_number()
{
    return wnck_workspace_get_number(this->workspace_);
}

std::string Workspace::get_name()
{
    RET_WRAP_NULL(wnck_workspace_get_name(this->workspace_));
}

void Workspace::change_name(const std::string &name)
{
    KLOG_PROFILE("name: %s.", name.c_str());

    return wnck_workspace_change_name(this->workspace_, name.c_str());
}

void Workspace::activate(uint32_t timestamp)
{
    KLOG_PROFILE("timestamp: %d.", timestamp);

    wnck_workspace_activate(this->workspace_, timestamp);
}

WindowVec Workspace::get_windows()
{
    WindowVec windows;

    flush_windows();

    for (auto iter = this->windows_.begin(); iter != this->windows_.end(); ++iter)
    {
        auto window = WindowManager::get_instance()->get_window(*iter);
        if (window)
        {
            windows.push_back(window);
        }
    }
    return windows;
}

void Workspace::flush_windows()
{
    KLOG_PROFILE("");

    for (auto iter = this->windows_.begin(); iter != this->windows_.end();)
    {
        auto window = WindowManager::get_instance()->get_window(*iter);

        if (!window)
        {
            this->windows_.erase(iter++);
            continue;
        }

        auto workspace = window->get_workspace();

        if (workspace && workspace->get_number() != this->get_number())
        {
            this->windows_.erase(iter++);
            continue;
        }
        else if (!workspace && !window->is_pinned())
        {
            this->windows_.erase(iter++);
            continue;
        }
        ++iter;
    }
}

void Workspace::add_window(std::shared_ptr<Window> window)
{
    KLOG_PROFILE("number: %d xid: %" PRIu64 ".", this->get_number(), window ? window->get_xid() : 0);

    auto xid = window->get_xid();
    if (this->windows_.find(xid) == this->windows_.end())
    {
        this->windows_.insert(window->get_xid());
        this->windows_changed_.emit();
    }
}

void Workspace::remove_window(std::shared_ptr<Window> window)
{
    KLOG_PROFILE("number: %d xid: %" PRIu64 ".", this->get_number(), window ? window->get_xid() : 0);

    auto xid = window->get_xid();

    if (this->windows_.find(xid) != this->windows_.end())
    {
        this->windows_.erase(window->get_xid());
        this->windows_changed_.emit();
    }
}

}  // namespace Kiran