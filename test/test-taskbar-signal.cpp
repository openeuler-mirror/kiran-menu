/*
 * @Author       : tangjie02
 * @Date         : 2020-05-20 20:12:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-11 17:27:49
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-taskbar-signal.cpp
 */
#include <gtkmm.h>

#include "lib/app.h"
#include "lib/core_worker.h"

void sig_hander(int signo)
{
}

static void fixed_app_added(Kiran::AppVec apps)
{
    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];
        g_print("recv fixed app %s added signal.\n", app->get_desktop_id().c_str());
    }
}

static void fixed_app_deleted(Kiran::AppVec apps)
{
    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];
        g_print("recv fixed app %s deleted signal.\n", app->get_desktop_id().c_str());
    }
}

int main(int argc, char **argv)
{
    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    auto taskbar = Kiran::TaskBarSkeleton::get_instance();

    taskbar->signal_fixed_app_added().connect(&fixed_app_added);
    taskbar->signal_fixed_app_deleted().connect(&fixed_app_deleted);

    kit.run();

    return 0;
}
