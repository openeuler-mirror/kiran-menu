/*
 * @Author       : tangjie02
 * @Date         : 2020-05-09 09:31:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:43:47
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-app-favorite.cpp
 */
#include "test/test-menu-common.h"

void test_favorite_apps(gconstpointer data)
{
    // Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;
    Kiran::MenuSkeleton *menu_skeleton = Kiran::MenuSkeleton::get_instance();

    g_print("\n---------------------------------------------\n");

    // delte all favorite apps
    auto favorite_apps = menu_skeleton->get_favorite_apps();

    for (int i = 0; i < favorite_apps.size(); ++i)
    {
        auto &app = favorite_apps[i];
        menu_skeleton->del_favorite_app(app->get_desktop_id());
    }

    // insert all apps to favorite app
    auto all_apps = menu_skeleton->get_all_sorted_apps();

    gboolean call_success;
    for (int i = 0; i < all_apps.size(); ++i)
    {
        auto &app = all_apps[i];
        menu_skeleton->add_favorite_app(app->get_desktop_id());
        g_assert_true(call_success);
    }

    // delete all favorite apps except last one.
    std::string last_desktop_id;

    for (int i = 0; i < all_apps.size(); ++i)
    {
        auto &app = all_apps[i];
        if (i + 1 == all_apps.size())
        {
            last_desktop_id = app->get_desktop_id();
        }
        else
        {
            call_success = menu_skeleton->del_favorite_app(app->get_desktop_id());
            g_assert_true(call_success);
        }
    }

    // check favorite app
    {
        auto app = menu_skeleton->lookup_favorite_app(last_desktop_id);
        g_assert_true(app != nullptr);
    }

    {
        favorite_apps = menu_skeleton->get_favorite_apps();
        gsize apps_len = 0;
        for (int i = 0; i < favorite_apps.size(); ++i)
        {
            auto &app = favorite_apps[i];
            ++apps_len;
            if (apps_len == 1)
            {
                g_assert_cmpstr(app->get_desktop_id().c_str(), ==, last_desktop_id.c_str());
            }
        }
        g_assert_true(apps_len == 1);
    }
}