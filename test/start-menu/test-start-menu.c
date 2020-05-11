/*
 * @Author       : tangjie02
 * @Date         : 2020-05-08 15:17:35
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 19:10:24
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

static void installed_changed(KiranMenuBased *based, gpointer user_data)
{
    g_print("recv installed-changed signal.\n");
}

static void app_installed(KiranMenuBased *based, gpointer user_data)
{
    GList *apps = (GList *)user_data;
    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        g_print("recv app %s installed signal.\n", kiran_app_get_desktop_id(app));
    }
}

static void app_uninstalled(KiranMenuBased *based, gpointer user_data)
{
    GList *apps = (GList *)user_data;
    for (GList *l = apps; l != NULL; l = l->next)
    {
        gchar *desktop_id = (gchar *)(l->data);
        g_print("recv app %s uninstalled signal.\n", desktop_id);
    }
}

static void favorite_app_added(KiranMenuBased *based, gpointer user_data)
{
    KiranApp *app = KIRAN_APP(user_data);
    g_print("recv favorite app %s added signal.\n", kiran_app_get_desktop_id(app));
}

static void favorite_app_deleted(KiranMenuBased *based, gpointer user_data)
{
    KiranApp *app = KIRAN_APP(user_data);
    g_print("recv favorite app %s deleted signal.\n", kiran_app_get_desktop_id(app));
}

static void frequent_usage_app_changed(KiranMenuBased *based, gpointer user_data)
{
    g_print("recv frequent-usage-app-changed signal.\n");
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    signal(SIGTRAP, sig_hander);

    gtk_init(&argc, &argv);

    KiranMenuBased *kiran_menu = kiran_menu_based_skeleton_get();

    g_signal_connect(kiran_menu, "app-changed", G_CALLBACK(installed_changed), NULL);
    g_signal_connect(kiran_menu, "app-installed", G_CALLBACK(app_installed), NULL);
    g_signal_connect(kiran_menu, "app-uninstalled", G_CALLBACK(app_uninstalled), NULL);
    g_signal_connect(kiran_menu, "favorite-app-added", G_CALLBACK(favorite_app_added), NULL);
    g_signal_connect(kiran_menu, "favorite-app-deleted", G_CALLBACK(favorite_app_deleted), NULL);
    g_signal_connect(kiran_menu, "frequent-usage-app-changed", G_CALLBACK(frequent_usage_app_changed), NULL);

    gtk_main();

    g_test_add_data_func("/test-start-menu/test-favorite-apps", kiran_menu,
                         test_favorite_apps);

    g_test_add_data_func("/test-start-menu/test-category-apps", kiran_menu,
                         test_category_apps);

    g_test_add_data_func("/test-start-menu/test-frequent-apps", kiran_menu,
                         test_frequent_apps);

    g_test_add_data_func("/test-start-menu/test-search-apps", kiran_menu,
                         test_search_apps);

    g_test_add_data_func("/test-start-menu/test-all-apps", kiran_menu,
                         test_all_apps);

    return g_test_run();
}
