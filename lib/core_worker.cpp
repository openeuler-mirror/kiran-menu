/*
 * @Author       : tangjie02
 * @Date         : 2020-06-10 17:15:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 14:37:54
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/core_worker.cpp
 */

#include "lib/core_worker.h"

namespace Kiran
{
class ScreenLogger : public Kiran::ILogger
{
public:
    void write_log(const char* buff, uint32_t len)
    {
        g_print(buff);
    }
};

void init_backend_system()
{
    Log::global_init();

    Kiran::Log::get_instance()->set_logger(new ScreenLogger());

    ScreenManager::global_init();

    WindowManager::global_init(ScreenManager::get_instance());

    AppManager::global_init(WindowManager::get_instance());

    WorkspaceManager::global_init(WindowManager::get_instance());

    MenuSkeleton::global_init(AppManager::get_instance());

    TaskBarSkeleton::global_init(AppManager::get_instance());

    ScreenManager::get_instance()->force_update();
}

void deinit_backend_system()
{
    TaskBarSkeleton::global_deinit();

    MenuSkeleton::global_deinit();

    WorkspaceManager::global_deinit();

    AppManager::global_deinit();

    WindowManager::global_deinit();

    ScreenManager::global_deinit();

    Log::global_deinit();
}

}  // namespace Kiran