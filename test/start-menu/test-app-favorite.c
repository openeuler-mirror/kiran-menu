#include "test/start-menu/test-start-menu.h"

void test_favorite_apps(gconstpointer data) {
  KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);

  GList *favorite_apps = kiran_menu_based_get_favorite_apps(menu_based);

  for (GList *l = favorite_apps; l != NULL; l = l->next) {
    KiranApp *app = l->data;
    kiran_menu_based_del_favorite_app(menu_based,
                                      kiran_app_get_desktop_id(app));
  }
  g_list_free_full(favorite_apps, g_object_unref);

  GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);

  gboolean call_success;
  for (GList *l = all_apps; l != NULL; l = l->next) {
    KiranApp *app = l->data;
    call_success = kiran_menu_based_add_favorite_app(
        menu_based, kiran_app_get_desktop_id(app));
    g_assert_true(call_success);
  }

  const gchar *last_desktop_id = NULL;

  for (GList *l = all_apps; l != NULL; l = l->next) {
    KiranApp *app = l->data;
    if (l->next == NULL) {
      last_desktop_id = kiran_app_get_desktop_id(app);
    } else {
      call_success = kiran_menu_based_del_favorite_app(
          menu_based, kiran_app_get_desktop_id(app));
      g_assert_true(call_success);
    }
  }

  favorite_apps = kiran_menu_based_get_favorite_apps(menu_based);
  gsize apps_len = 0;
  for (GList *l = favorite_apps; l != NULL; l = l->next) {
    KiranApp *app = l->data;
    ++apps_len;
    if (apps_len == 1) {
      g_assert_cmpstr(kiran_app_get_desktop_id(app), ==, last_desktop_id);
    }
  }
  g_assert_true(apps_len == 1);

  g_list_free_full(favorite_apps, g_object_unref);

  g_list_free_full(all_apps, g_object_unref);
}