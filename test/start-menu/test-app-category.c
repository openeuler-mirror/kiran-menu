#include "test/start-menu/test-start-menu.h"

#define ADD_CATEGORY_SYNC(category, check_ret)                \
  {                                                           \
    call_success = kiran_start_menu_s_call_add_category_sync( \
        proxy, category, &out_b1, NULL, &error);              \
    CHECK_PROXY_CALL_ERR(call_success, error);                \
    g_assert_true(!check_ret || out_b1);                      \
  }

#define DEL_CATEGORY_SYNC(category, check_ret)                \
  {                                                           \
    call_success = kiran_start_menu_s_call_del_category_sync( \
        proxy, category, &out_b1, NULL, &error);              \
    CHECK_PROXY_CALL_ERR(call_success, error);                \
    g_assert_true(!check_ret || out_b1);                      \
  }

#define ADD_CATEGORY_APP_SYNC(category, desktop_file)             \
  {                                                               \
    call_success = kiran_start_menu_s_call_add_category_app_sync( \
        proxy, category, desktop_file, &out_b1, NULL, &error);    \
    CHECK_PROXY_CALL_ERR(call_success, error);                    \
    g_assert_true(out_b1);                                        \
  }

#define DEL_CATEGORY_APP_SYNC(category, desktop_file)             \
  {                                                               \
    call_success = kiran_start_menu_s_call_del_category_app_sync( \
        proxy, category, desktop_file, &out_b1, NULL, &error);    \
    CHECK_PROXY_CALL_ERR(call_success, error);                    \
    g_assert_true(out_b1);                                        \
  }

void test_category_apps(gconstpointer data) {
  GError *error = NULL;
  KiranStartMenuS *proxy = KIRAN_START_MENU_S(data);
  gboolean call_success;
  gboolean out_b1;

  DEL_CATEGORY_SYNC("category_test1", FALSE);
  ADD_CATEGORY_SYNC("category_test1", TRUE);

  ADD_CATEGORY_APP_SYNC("category_test1", "app1");
  ADD_CATEGORY_APP_SYNC("category_test1", "app2");
  ADD_CATEGORY_APP_SYNC("category_test1", "app3");
  DEL_CATEGORY_APP_SYNC("category_test1", "app2");

  gchar **out_apps;
  call_success = kiran_start_menu_s_call_get_category_apps_sync(
      proxy, "category_test1", &out_apps, NULL, &error);
  CHECK_PROXY_CALL_ERR(call_success, error);
  gsize apps_len = g_strv_length(out_apps);
  g_assert_true(apps_len == 2);
  g_assert_cmpstr(out_apps[0], ==, "app1");
  g_assert_cmpstr(out_apps[1], ==, "app3");

  ADD_CATEGORY_SYNC("category_test2", FALSE);
  DEL_CATEGORY_SYNC("category_test2", TRUE);
}