/*
 * @Author       : tangjie02
 * @Date         : 2020-05-11 15:06:43
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 08:57:17
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-app-frequent.cpp
 */
#include "test/test-menu-common.h"

#define CHECK_GET_BOUNDARY(n)                             \
    {                                                     \
        auto apps = menu_skeleton->get_nfrequent_apps(n); \
    }

void test_frequent_apps(gconstpointer data)
{
    Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;

    g_print("\n---------------------------------------------\n");

    CHECK_GET_BOUNDARY(0);
    CHECK_GET_BOUNDARY(1);
    CHECK_GET_BOUNDARY(3);
    CHECK_GET_BOUNDARY(1000);

    auto apps = menu_skeleton->get_nfrequent_apps(5);

    g_print("frequent_apps: ");
    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];
        g_print("%s ", app->get_desktop_id().c_str());
    }
    g_print("\n");
}