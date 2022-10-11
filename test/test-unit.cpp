/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include <gtkmm.h>

#include "lib/core_worker.h"
#include "test/test-menu-common.h"

void test_favorite_apps(gconstpointer data)
{
    auto menu_skeleton = Kiran::MenuSkeleton::get_instance();

    g_print("\n---------------------------------------------\n");

    // delte all favorite apps
    auto favorite_apps = menu_skeleton->get_favorite_apps();

    for (auto app : favorite_apps)
    {
        menu_skeleton->del_favorite_app(app->get_desktop_id());
    }

    // insert all apps to favorite app
    auto all_apps = menu_skeleton->get_all_sorted_apps();

    gboolean call_success = false;

    for (auto app : all_apps)
    {
        call_success = menu_skeleton->add_favorite_app(app->get_desktop_id());
        g_assert_true(call_success);
    }

    // delete all favorite apps except last one.
    std::string last_desktop_id;

    for (int i = 0; i < (int)all_apps.size(); ++i)
    {
        auto app = all_apps[i];
        if (i + 1 == (int)all_apps.size())
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

        for (auto app : favorite_apps)
        {
            ++apps_len;
            if (apps_len == 1)
            {
                g_assert_cmpstr(app->get_desktop_id().c_str(), ==, last_desktop_id.c_str());
            }
        }
        g_assert_true(apps_len == 1);
    }
}

void test_category_apps(gconstpointer data)
{
    auto menu_skeleton = Kiran::MenuSkeleton::get_instance();

    auto all_category_apps = menu_skeleton->get_all_category_apps();

    std::string first_category_name;

    g_print("\n---------------------------------------------\n");
    g_print("category info:\n");

    for (auto iter = all_category_apps.begin(); iter != all_category_apps.end(); ++iter)
    {
        if (first_category_name.empty())
        {
            first_category_name = iter->first;
        }
        g_print("category_name: %s\n", iter->first.c_str());
        g_print("category_apps: ");

        for (int i = 0; i < (int)iter->second.size(); ++i)
        {
            auto &app = iter->second[i];
            g_print("%s ", app->get_desktop_id().c_str());
        }
        g_print("\n");
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

#define CHECK_GET_BOUNDARY(n)                             \
    {                                                     \
        auto apps = menu_skeleton->get_nfrequent_apps(n); \
    }

void test_frequent_apps(gconstpointer data)
{
    auto menu_skeleton = Kiran::MenuSkeleton::get_instance();

    g_print("\n---------------------------------------------\n");

    CHECK_GET_BOUNDARY(0);
    CHECK_GET_BOUNDARY(1);
    CHECK_GET_BOUNDARY(3);
    CHECK_GET_BOUNDARY(1000);

    auto apps = menu_skeleton->get_nfrequent_apps(5);

    g_print("frequent_apps: ");

    for (auto app : apps)
    {
        g_print("%s ", app->get_desktop_id().c_str());
    }
    g_print("\n");
}

#define CHECK_SEARCH_RESULT(keyword)                                                    \
    {                                                                                   \
        auto search_apps = menu_skeleton->search_app(keyword);                          \
        g_assert_true((int)search_apps.size() == 1);                                    \
        auto &s_app = search_apps[0];                                                   \
        g_assert_cmpstr(first_desktop_id.c_str(), ==, s_app->get_desktop_id().c_str()); \
    }

void test_search_apps(gconstpointer data)
{
    auto app_manager = Kiran::AppManager::get_instance();
    auto menu_skeleton = Kiran::MenuSkeleton::get_instance();

    g_print("\n---------------------------------------------\n");

    auto all_apps = app_manager->get_should_show_apps();

    if ((int)all_apps.size() > 0)
    {
        auto &first_app = all_apps[0];
        auto first_desktop_id = first_app->get_desktop_id();

        CHECK_SEARCH_RESULT(first_app->get_name());
        CHECK_SEARCH_RESULT(first_app->get_locale_name());
        CHECK_SEARCH_RESULT(first_app->get_comment());
        CHECK_SEARCH_RESULT(first_app->get_locale_comment());
    }

    // GList *tt = kiran_menu_based_search_app_ignore_case(menu_based, "fIre");
    // for (GList *l = tt; l != NULL; l = l->next)
    // {
    //     g_print("id: %s\n", kiran_app_get_desktop_id(l->data));
    // }
}

void test_fixed_apps(gconstpointer data)
{
    auto taskbar_skeleton = Kiran::TaskBarSkeleton::get_instance();
    auto app_manager = Kiran::AppManager::get_instance();

    g_print("\n---------------------------------------------\n");

    // delte all fixed apps
    auto fixed_apps = taskbar_skeleton->get_fixed_apps();

    for (auto app : fixed_apps)
    {
        taskbar_skeleton->del_fixed_app(app->get_desktop_id());
    }

    // insert all apps to fixed app
    auto all_apps = app_manager->get_apps();

    gboolean call_success = true;

    for (auto app : all_apps)
    {
        taskbar_skeleton->add_fixed_app(app->get_desktop_id());
        g_assert_true(call_success);
    }

    // delete all fixed apps except last one.
    std::string last_desktop_id;

    for (int i = 0; i < (int)all_apps.size(); ++i)
    {
        auto app = all_apps[i];
        if (i + 1 == (int)all_apps.size())
        {
            last_desktop_id = app->get_desktop_id();
        }
        else
        {
            call_success = taskbar_skeleton->del_fixed_app(app->get_desktop_id());
            g_assert_true(call_success);
        }
    }
    g_assert_true(taskbar_skeleton->get_fixed_apps().size() <= 1);

    // check fixed app
    {
        auto app = taskbar_skeleton->lookup_fixed_app(last_desktop_id);
        g_assert_true(app != nullptr);
    }

    {
        fixed_apps = taskbar_skeleton->get_fixed_apps();
        gsize apps_len = 0;
        for (auto app : fixed_apps)
        {
            ++apps_len;
            if (apps_len == 1)
            {
                g_assert_cmpstr(app->get_desktop_id().c_str(), ==, last_desktop_id.c_str());
            }
        }
        g_assert_true(apps_len == 1);
    }
}

void sig_hander(int signo)
{
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    g_test_add_data_func("/test-startmenu/test-favorite-apps", NULL,
                         test_favorite_apps);

    g_test_add_data_func("/test-startmenu/test-category-apps", NULL,
                         test_category_apps);

    g_test_add_data_func("/test-startmenu/test-frequent-apps", NULL,
                         test_frequent_apps);

    g_test_add_data_func("/test-startmenu/test-search-apps", NULL,
                         test_search_apps);

    g_test_add_data_func("/test-taskbar/test-fixed-apps", NULL,
                         test_fixed_apps);

    g_test_run();

    kit.run();
    return 0;
}
