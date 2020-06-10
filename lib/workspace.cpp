/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:04
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 09:28:57
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace.cpp
 */

#include "lib/workspace.h"

namespace Kiran
{
Workspace::Workspace(WnckWorkspace *workspace) : workspace_(workspace)
{
}

Workspace::~Workspace()
{
}

WindowVec Workspace::get_windows()
{
    WindowVec windows;
    for (auto iter = this->windows_.begin(); iter != this->windows_.end(); ++iter)
    {
        auto &window = (*iter);
        if (!window.expired())
        {
            windows.push_back(window.lock());
        }
    }
    return windows;
}
}  // namespace Kiran