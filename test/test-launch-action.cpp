/*
 * @Author       : tangjie02
 * @Date         : 2020-06-12 13:52:35
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-12 13:56:03
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-launch-action.cpp
 */

#include <gtkmm.h>

#include "lib/app.h"
#include "lib/core_worker.h"

void sig_hander(int signo)
{
}

int main(int argc, char **argv)
{
    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    auto app = Kiran::AppManager::get_instance()->lookup_app("firefox.desktop");
    auto actions = app->get_actions();
    if (actions.size() > 0)
    {
        app->launch_action(actions[0]);
    }

    kit.run();

    return 0;
}
