/*
 * @Author       : tangjie02
 * @Date         : 2020-06-10 17:15:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 17:41:31
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/core_worker.cpp
 */

#include "lib/core_worker.h"

namespace Kiran
{
void init_backend_system()
{
    WindowManager::global_init();

    AppManager::global_init(WindowManager::get_instance());

    WorkspaceManager::global_init();

    MenuSkeleton::global_init();
}

void deinit_backend_system()
{
    MenuSkeleton::global_deinit();

    WorkspaceManager::global_deinit();

    AppManager::global_deinit();

    WindowManager::global_deinit();
}

}  // namespace Kiran