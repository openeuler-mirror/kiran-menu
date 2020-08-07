/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:55:52
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-07 14:06:48
 * @Description  : 该类是对WnckWorkspace的封装，大部分接口和wnck_window_xxxx相同。
 * @FilePath     : /kiran-menu-2.0/lib/workspace.h
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

private:
    void flush_windows();

    // 添加窗口
    void add_window(std::shared_ptr<Window> window);
    //移除窗口
    void remove_window(std::shared_ptr<Window> window);

private:
    WnckWorkspace *workspace_;

    std::set<uint64_t> windows_;

    friend class Window;
};
}  // namespace Kiran