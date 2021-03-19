/**
 * @file          /kiran-menu/test/test-taskbar-signal.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <gtkmm.h>

#include "lib/app.h"
#include "lib/core_worker.h"

void sig_hander(int signo)
{
}

static void fixed_app_added(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv fixed app %s added signal.\n", app->get_desktop_id().c_str());
    }
}

static void fixed_app_deleted(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
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
