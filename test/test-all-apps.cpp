/**
 * @file          /kiran-menu/test/test-all-apps.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <gtkmm.h>

#include "lib/app.h"
#include "lib/core_worker.h"
#include "test/test-menu-common.h"

#define CASES 10000

void print_apps(const Kiran::AppVec &apps, const std::string &type)
{
    g_print("\n--------------------- %s ------------------------\n", type.c_str());

    for (auto app : apps)
    {
        g_print("desktop_id: %s\n", app->get_desktop_id().c_str());
    }
    g_print("\n");
}

void timing_print(gconstpointer data)
{
    auto menu_skeleton = Kiran::MenuSkeleton::get_instance();
    auto taskbar_skeleton = Kiran::TaskBarSkeleton::get_instance();
    auto app_manager = Kiran::AppManager::get_instance();

    {
        g_print("\n-------------------------- all apps-----------------------\n");
        auto all_apps = app_manager->get_apps();
        for (auto app : all_apps)
        {
            g_print("desktop_id: %s\n", app->get_desktop_id().c_str());

            auto actions = app->get_actions();
            for (int j = 0; j < (int)actions.size(); ++j)
            {
                g_print("   action/name: %s/%s\n", actions[j].c_str(), app->get_action_name(actions[j]).c_str());
            }
            g_print("\n");
        }
        g_print("\n");
    }

    print_apps(menu_skeleton->get_nnew_apps(-1), "new apps");
    print_apps(menu_skeleton->get_favorite_apps(), "favorite apps");
    print_apps(taskbar_skeleton->get_fixed_apps(), "fixed apps");

    gint64 start_clock = g_get_real_time();
    for (int i = 0; i <= CASES; ++i)
    {
        auto all_apps = menu_skeleton->get_all_sorted_apps();
    }
    gint64 end_clock = g_get_real_time();

    auto all_apps = menu_skeleton->get_all_sorted_apps();

    g_print("run %d cases cost %f seconds. sort length: %d \n", CASES,
            (end_clock - start_clock) * 1.0 / G_TIME_SPAN_SECOND, (int)all_apps.size());
}

void sig_hander(int signo)
{
}

int main(int argc, char **argv)
{
    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    timing_print(NULL);

    // g_timeout_add_seconds(10, timing_print, NULL);

    kit.run();

    return 0;
}