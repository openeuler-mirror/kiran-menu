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