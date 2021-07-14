/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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