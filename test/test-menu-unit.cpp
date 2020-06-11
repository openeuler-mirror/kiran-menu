/*
 * @Author       : tangjie02
 * @Date         : 2020-05-08 15:17:35
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 09:51:46
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-menu-unit.cpp
 */
#include <gtkmm.h>

#include "lib/core_worker.h"
#include "test/test-menu-common.h"

void test_favorite_apps(gconstpointer data);
void test_category_apps(gconstpointer data);
void test_frequent_apps(gconstpointer data);
void test_search_apps(gconstpointer data);
void test_all_apps(gconstpointer data);

void sig_hander(int signo)
{
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    Kiran::MenuSkeleton *menu_skeleton = Kiran::MenuSkeleton::get_instance();

    g_test_add_data_func("/test-start-menu/test-favorite-apps", NULL,
                         test_favorite_apps);

    g_test_add_data_func("/test-start-menu/test-category-apps", menu_skeleton,
                         test_category_apps);

    g_test_add_data_func("/test-start-menu/test-frequent-apps", menu_skeleton,
                         test_frequent_apps);

    g_test_add_data_func("/test-start-menu/test-search-apps", menu_skeleton,
                         test_search_apps);

    g_test_add_data_func("/test-start-menu/test-all-apps", menu_skeleton,
                         test_all_apps);

    return g_test_run();
}
