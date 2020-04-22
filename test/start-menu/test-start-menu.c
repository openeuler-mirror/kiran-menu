#include "test/start-menu/test-start-menu.h"

#include "src/start-menu/kiran-menu-based.h"

void test_favorite_apps(gconstpointer data);
void test_category_apps(gconstpointer data);
void test_frequent_apps(gconstpointer data);
void test_search_apps(gconstpointer data);
void test_all_apps(gconstpointer data);

int main(int argc, char **argv) {
  KiranMenuBased *kiran_menu = kiran_menu_based_skeleton_new();

  g_test_init(&argc, &argv, NULL);

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
