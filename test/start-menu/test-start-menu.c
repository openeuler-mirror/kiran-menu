/*
 * @Author       : tangjie02
 * @Date         : 2020-05-08 15:17:35
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 15:56:12
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/start-menu/test-start-menu.c
 */
#include "test/start-menu/test-start-menu.h"

#include <gtk/gtk.h>

#include "lib/kiran-menu-based.h"

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

    gtk_init(&argc, &argv);

    KiranMenuBased *kiran_menu = kiran_menu_based_skeleton_get();

    //gtk_main();

    g_test_add_data_func("/test-start-menu/test-favorite-apps", kiran_menu,
                         test_favorite_apps);

    g_test_add_data_func("/test-start-menu/test-category-apps", kiran_menu,
                         test_category_apps);

    // g_test_add_data_func("/test-start-menu/test-frequent-apps", kiran_menu,
    //                      test_frequent_apps);

    g_test_add_data_func("/test-start-menu/test-search-apps", kiran_menu,
                         test_search_apps);

    g_test_add_data_func("/test-start-menu/test-all-apps", kiran_menu,
                         test_all_apps);

    return g_test_run();
}
