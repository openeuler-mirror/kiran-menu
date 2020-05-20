/*
 * @Author       : tangjie02
 * @Date         : 2020-05-11 15:06:43
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-20 20:09:43
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-app-frequent.c
 */
#include "test/test-menu-common.h"

#define CHECK_GET_BOUNDARY(n)                                             \
    {                                                                     \
        GList *apps = kiran_menu_based_get_nfrequent_apps(menu_based, n); \
        g_list_free_full(apps, g_object_unref);                           \
    }

void test_frequent_apps(gconstpointer data)
{
    KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);

    g_print("\n---------------------------------------------\n");

    CHECK_GET_BOUNDARY(0);
    CHECK_GET_BOUNDARY(1);
    CHECK_GET_BOUNDARY(3);
    CHECK_GET_BOUNDARY(1000);

    GList *apps = kiran_menu_based_get_nfrequent_apps(menu_based, 5);

    g_print("frequent_apps: ");
    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        g_print("%s ", kiran_app_get_desktop_id(app));
    }
    g_print("\n");
}