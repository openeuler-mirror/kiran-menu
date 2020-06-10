/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:55:52
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 17:43:28
 * @Description  : 该类是对WnckWorkspace的封装，大部分接口和wnck_window_xxxx相同。
 * @FilePath     : /kiran-menu-2.0/lib/workspace.h
 */

#pragma once

#include <libwnck/libwnck.h>

#include "lib/window.h"

namespace Kiran
{
class Workspace;
class Window;

using WorkspaceVec = std::vector<std::shared_ptr<Workspace>>;

class Workspace
{
   public:
    Workspace(WnckWorkspace *workspace);
    virtual ~Workspace();

    // 获取工作空间中的窗口
    WindowVec get_windows();

   private:
    WnckWorkspace *workspace_;
    std::vector<std::weak_ptr<Window>> windows_;
    friend class Window;
};
}  // namespace Kiran