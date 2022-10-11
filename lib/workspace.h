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

#pragma once

#include <libwnck/libwnck.h>

#include <set>

#include "lib/window.h"

namespace Kiran
{
class Workspace;
class Window;

using WorkspaceVec = std::vector<std::shared_ptr<Kiran::Workspace>>;

// 该类是对WnckWorkspace的封装，大部分接口和wnck_window_xxxx相同。
class Workspace
{
public:
    Workspace(WnckWorkspace *workspace);
    virtual ~Workspace();

    // 获取工作区索引，从0开始
    int get_number();

    // 获取工作区名字
    std::string get_name();

    // 修改工作区名字
    void change_name(const std::string &name);

    // 激活工作区
    void activate(uint32_t timestamp);

    // 获取工作空间中的窗口
    WindowVec get_windows();

    // 工作区中的窗口列表发生变化的信号
    sigc::signal<void> signal_windows_changes() { return this->windows_changed_; };

private:
    void flush_windows();

    // 添加窗口
    void add_window(std::shared_ptr<Window> window);
    //移除窗口
    void remove_window(std::shared_ptr<Window> window);

private:
    WnckWorkspace *workspace_;

    std::set<uint64_t> windows_;

    sigc::signal<void> windows_changed_;

    friend class Window;
    friend class WorkspaceManager;
};
}  // namespace Kiran