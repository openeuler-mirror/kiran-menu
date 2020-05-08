/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 16:33:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-08 15:39:48
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/start-menu/test-app-category.c
 */
#include <glib.h>
#include <glib/gprintf.h>

#include "lib/kiran-app.h"
#include "lib/kiran-menu-skeleton.h"
#include "test/start-menu/test-start-menu.h"

void test_category_apps(gconstpointer data)
{
    KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);

    gboolean call_success;

    GHashTable *all_category_apps = kiran_menu_based_get_all_category_apps(menu_based);

    GHashTableIter iter;
    gchar *category_name = NULL;
    GList *category_apps = NULL;

    g_autofree gchar *first_category_name = NULL;

    g_hash_table_iter_init(&iter, all_category_apps);

    g_printf("\ncategory info:\n");

    while (g_hash_table_iter_next(&iter, (gpointer *)&category_name, (gpointer *)&category_apps))
    {
        if (!first_category_name)
        {
            first_category_name = g_strdup(category_name);
        }
        g_printf("category_name: %s\n", category_name);
        g_printf("category_apps: ");

        for (GList *l = category_apps; l != NULL; l = l->next)
        {
            KiranApp *app = l->data;
            const gchar *desktop_id = kiran_app_get_desktop_id(app);
            g_printf("%s ", desktop_id);
        }
        g_printf("\n");
    }

    g_hash_table_unref(all_category_apps);

    if (first_category_name)
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