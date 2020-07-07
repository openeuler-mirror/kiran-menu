/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 17:36:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-07 11:27:28
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-all-apps.cpp
 */

#include "lib/menu-skeleton.h"
#include "test/test-menu-common.h"

#define CASES 10000

void test_all_apps(gconstpointer data)
{
    Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;

    {
        g_print("\n-------------------------- all apps-----------------------\n");
        auto all_apps = menu_skeleton->get_all_sorted_apps();
        for (int i = 0; i < all_apps.size(); ++i)
        {
            auto &app = all_apps[i];
            g_print("desktop_id: %s\n", app->get_desktop_id().c_str());

            auto actions = app->get_actions();
            for (int j = 0; j < actions.size(); ++j)
            {
                g_print("   action/name: %s/%s\n", actions[j].c_str(), app->get_action_name(actions[j]).c_str());
            }
            g_print("\n");
        }
        g_print("\n");
    }

    {
        g_print("\n--------------------- new apps ------------------------\n");
        auto new_apps = menu_skeleton->get_nnew_apps(-1);

        for (int i = 0; i < new_apps.size(); ++i)
        {
            auto &app = new_apps[i];

            g_print("desktop_id: %s\n", app->get_desktop_id().c_str());
        }
        g_print("\n");
    }

    gint64 start_clock = g_get_real_time();
    for (int i = 0; i <= CASES; ++i)
    {
        auto all_apps = menu_skeleton->get_all_sorted_apps();
    }
    gint64 end_clock = g_get_real_time();

    auto all_apps = menu_skeleton->get_all_sorted_apps();

    g_print("run %d cases cost %f seconds. sort length: %d \n", CASES,
            (end_clock - start_clock) * 1.0 / G_TIME_SPAN_SECOND, all_apps.size());
}