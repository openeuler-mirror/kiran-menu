/*
 * @Author       : tangjie02
 * @Date         : 2020-05-09 10:40:16
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 08:59:46
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-app-search.cpp
 */
#include <glib/gprintf.h>

#include "test/test-menu-common.h"

#define CHECK_SEARCH_RESULT(keyword)                                                    \
    {                                                                                   \
        auto search_apps = menu_skeleton->search_app(keyword);                          \
        g_assert_true(search_apps.size() == 1);                                         \
        auto &s_app = search_apps[0];                                                   \
        g_assert_cmpstr(first_desktop_id.c_str(), ==, s_app->get_desktop_id().c_str()); \
    }

void test_search_apps(gconstpointer data)
{
    Kiran::MenuSkeleton *menu_skeleton = (Kiran::MenuSkeleton *)data;

    g_print("\n---------------------------------------------\n");

    auto all_apps = menu_skeleton->get_all_sorted_apps();

    if (all_apps.size() > 0)
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