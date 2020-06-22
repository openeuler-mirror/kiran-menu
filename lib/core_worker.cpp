/*
 * @Author       : tangjie02
 * @Date         : 2020-06-10 17:15:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-19 18:24:21
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/core_worker.cpp
 */

#include "lib/core_worker.h"

namespace Kiran
{
void init_backend_system()
{
    ScreenManager::global_init();

    WindowManager::global_init(ScreenManager::get_instance());

    AppManager::global_init(WindowManager::get_instance());

    WorkspaceManager::global_init();

    MenuSkeleton::global_init();

    ScreenManager::get_instance()->force_update();
}

void deinit_backend_system()
{
    MenuSkeleton::global_deinit();

    WorkspaceManager::global_deinit();

    AppManager::global_deinit();

    WindowManager::global_deinit();

    ScreenManager::global_deinit();
}

}  // namespace Kiran