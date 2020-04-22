#include "test/start-menu/test-start-menu.h"

#define CHECK_SEARCH_RESULT(keyword)                                        \
  {                                                                         \
    GList *search_apps = kiran_menu_based_search_app(menu_based, keyword);  \
    g_assert_true(search_apps != NULL);                                     \
    g_assert_true(search_apps->next == NULL);                               \
    KiranApp *s_app = search_apps->data;                                    \
    g_assert_cmpstr(first_desktop_id, ==, kiran_app_get_desktop_id(s_app)); \
  }

void test_search_apps(gconstpointer data) {
  KiranMenuBased *menu_based = KIRAN_MENU_BASED((gpointer)data);
  GList *all_apps = kiran_menu_based_get_all_sorted_apps(menu_based);

  if (all_apps != NULL) {
    KiranApp *first_app = all_apps->data;
    const gchar *first_desktop_id = kiran_app_get_desktop_id(first_app);

    CHECK_SEARCH_RESULT(kiran_app_get_name(first_app));
    CHECK_SEARCH_RESULT(kiran_app_get_locale_name(first_app));
    CHECK_SEARCH_RESULT(kiran_app_get_comment(first_app));
    CHECK_SEARCH_RESULT(kiran_app_get_locale_comment(first_app));
  }
}