#include "test/start-menu/test-start-menu.h"

void test_category_apps(gconstpointer data) {
  KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);
  gboolean call_success;

  GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);

  if (all_apps != NULL) {
    KiranApp *first_app = all_apps->data;
    const gchar *first_desktop_id = kiran_app_get_desktop_id(first_app);

    kiran_menu_based_del_category_app(menu_based, "category_test1",
                                      first_desktop_id);

    call_success = kiran_menu_based_add_category_app(
        menu_based, "category_test1", first_desktop_id);
    g_assert_true(call_success);

    GList *category_apps =
        kiran_menu_based_get_category_apps(menu_based, "category_test1");

    g_assert_true(category_apps != NULL);
    g_assert_true(category_apps->next == NULL);

    g_assert_cmpstr(kiran_app_get_desktop_id(category_apps->data), ==,
                    first_desktop_id);

    g_list_free_full(category_apps, g_object_unref);

    call_success = kiran_menu_based_del_category_app(
        menu_based, "category_test1", first_desktop_id);
    g_assert_true(call_success);
  }
}