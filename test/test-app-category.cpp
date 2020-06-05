/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 16:33:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 08:56:04
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-app-category.cpp
 */
#include <glib.h>
#include <glib/gprintf.h>

#include "lib/app.h"
#include "lib/menu-skeleton.h"
#include "test/test-menu-common.h"

void test_category_apps(gconstpointer data)
{
    Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;

    gboolean call_success;

    auto all_category_apps = menu_skeleton->get_all_category_apps();

    std::string first_category_name;

    g_print("\n---------------------------------------------\n");
    g_printf("category info:\n");

    for (auto iter = all_category_apps.begin(); iter != all_category_apps.end(); ++iter)
    {
        if (first_category_name.empty())
        {
            first_category_name = iter->first;
        }
        g_printf("category_name: %s\n", iter->first.c_str());
        g_printf("category_apps: ");

        for (int i = 0; i < iter->second.size(); ++i)
        {
            auto &app = iter->second[i];
            g_printf("%s ", app->get_desktop_id().c_str());
        }
        g_printf("\n");
    }

    if (first_category_name.length() > 0)
    {
        // category_apps = kiran_menu_based_get_category_apps(menu_based, first_category_name);
        // if (category_apps)
        // {
        //     KiranApp *app = category_apps->data;
        //     kiran_menu_based_del_category_app(menu_based, first_category_name, kiran_app_get_desktop_id(app));
        // }

        // GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);
        // if (all_apps)
        // {
        //     KiranApp *app = all_apps->data;
        //     kiran_menu_based_add_category_app(menu_based, first_category_name, kiran_app_get_desktop_id(app));
        // }
    }
}

// void test_category_apps(gconstpointer data)
// {
//     KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);

//     test_category_apps2(data);
//     g_usleep(5000000);
//     kiran_menu_skeleton_flush(KIRAN_MENU_SKELETON(menu_based));
//     test_category_apps2(data);
// }