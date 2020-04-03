#include "test/start-menu/test-start-menu.h"

#define ADD_CATEGORY_APP_SYNC(category, desktop_file, check_ret)  \
  {                                                               \
    call_success = kiran_start_menu_s_call_add_category_app_sync( \
        proxy, category, desktop_file, &out_b1, NULL, &error);    \
    CHECK_PROXY_CALL_ERR(call_success, error);                    \
    g_assert_true(out_b1 == check_ret);                           \
  }

#define DEL_CATEGORY_APP_SYNC(category, desktop_file, check_ret)  \
  {                                                               \
    call_success = kiran_start_menu_s_call_del_category_app_sync( \
        proxy, category, desktop_file, &out_b1, NULL, &error);    \
    CHECK_PROXY_CALL_ERR(call_success, error);                    \
    g_assert_true(out_b1 == check_ret);                           \
  }

void test_category_apps(gconstpointer data) {
  GError *error = NULL;
  KiranStartMenuS *proxy = KIRAN_START_MENU_S(data);
  gboolean call_success;
  gboolean out_b1;

  ADD_CATEGORY_APP_SYNC("category_test1", "app1", FALSE);
  DEL_CATEGORY_APP_SYNC("category_test1", "app2", FALSE);

  gchar **sorted_apps;
  call_success = kiran_start_menu_s_call_get_all_sorted_apps_sync(
      proxy, &sorted_apps, NULL, &error);
  CHECK_PROXY_CALL_ERR(call_success, error);

  if (g_strv_length(sorted_apps) > 0) {
    DEL_CATEGORY_APP_SYNC("category_test1", sorted_apps[0], FALSE);
    ADD_CATEGORY_APP_SYNC("category_test1", sorted_apps[0], TRUE);
  }

  gchar **category_apps = NULL;
  call_success = kiran_start_menu_s_call_get_category_apps_sync(
      proxy, "category_test1", &category_apps, NULL, &error);
  CHECK_PROXY_CALL_ERR(call_success, error);

  gboolean exist_first_sort_app = FALSE;
  for (gint i = 0; category_apps[i] != NULL; ++i) {
    if (g_strcmp0(category_apps[i], sorted_apps[0]) == 0) {
      exist_first_sort_app = TRUE;
    }
  }
  g_assert_true(exist_first_sort_app);

  // g_print("s %s\n", sorted_apps[0]);

  DEL_CATEGORY_APP_SYNC("category_test1", sorted_apps[0], TRUE);
  DEL_CATEGORY_APP_SYNC("category_test1", sorted_apps[0], FALSE);

  GVariant *all_category_apps;
  call_success = kiran_start_menu_s_call_get_all_category_apps_sync(
      proxy, &all_category_apps, NULL, &error);
  CHECK_PROXY_CALL_ERR(call_success, error);
}