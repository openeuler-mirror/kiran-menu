#include "test/start-menu/test-start-menu.h"

void test_favorite_apps(gconstpointer data) {
  GError *error = NULL;
  KiranStartMenuS *proxy = KIRAN_START_MENU_S(data);

  gchar *empty_strv[] = {NULL};
  kiran_start_menu_s_set_favorite_apps(proxy, (const char *const *)empty_strv);

  const int kAppMaxLen = 10;
  gchar app[kAppMaxLen];
  gboolean call_success;
  gboolean call_output;
  for (guint i = 0; i < 10; ++i) {
    g_snprintf(app, kAppMaxLen, "apple%u", i);
    call_success = kiran_start_menu_s_call_add_favorite_app_sync(
        proxy, app, &call_output, NULL, &error);
    CHECK_PROXY_CALL_ERR(call_success, error);
    g_assert_true(call_output);
  }

  for (guint i = 0; i < 10; ++i) {
    if (i == 5 || i == 6) {
      continue;
    }
    g_snprintf(app, kAppMaxLen, "apple%u", i);
    call_success = kiran_start_menu_s_call_del_favorite_app_sync(
        proxy, app, &call_output, NULL, &error);
    CHECK_PROXY_CALL_ERR(call_success, error);
    g_assert_true(call_output);
  }

  const gchar *const *apps = kiran_start_menu_s_get_favorite_apps(proxy);
  gsize apps_len = 0;
  while (apps[apps_len]) {
    ++apps_len;
  }
  g_assert_true(apps_len == 2);
  g_assert_cmpstr(apps[0], ==, "apple5");
  g_assert_cmpstr(apps[1], ==, "apple6");
}