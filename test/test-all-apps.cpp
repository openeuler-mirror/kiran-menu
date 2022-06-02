/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 17:36:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 08:55:16
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-all-apps.cpp
 */

#include "lib/menu-skeleton.h"
#include "test/test-menu-common.h"

#define CASES 10000

void test_all_apps(gconstpointer data)
{
    Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;

    auto new_apps = menu_skeleton->get_nnew_apps(-1);

    g_print("\n---------------------------------------------\n");

    g_print("new apps: ");
    for (int i = 0; i < new_apps.size(); ++i)
    {
        auto &app = new_apps[i];
        g_print("%s ", app->get_desktop_id().c_str());
    }
    g_print("\n");

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