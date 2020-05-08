/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 17:36:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-08 15:28:13
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/start-menu/test-all-apps.c
 */

#include "lib/kiran-menu-skeleton.h"
#include "test/start-menu/test-start-menu.h"

#define CASES 10000

void test_all_apps(gconstpointer data)
{
    KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);

    gint64 start_clock = g_get_real_time();
    for (int i = 0; i <= CASES; ++i)
    {
        GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);
        g_list_free_full(all_apps, g_object_unref);
    }
    gint64 end_clock = g_get_real_time();

    GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);
    gint app_num = 0;
    for (GList *l = all_apps; l != NULL; l = l->next)
    {
        ++app_num;
    }
    g_print("run %d cases cost %f seconds. sort length: %d ", CASES,
            (end_clock - start_clock) * 1.0 / G_TIME_SPAN_SECOND, app_num);
    g_list_free_full(all_apps, g_object_unref);
}