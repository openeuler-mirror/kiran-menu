#include "test/start-menu/test-start-menu.h"

#define RESET_FREQUENT_APPS()                                        \
  {                                                                  \
    call_success = kiran_start_menu_s_call_reset_frequent_apps_sync( \
        proxy, &out_b1, NULL, &error);                               \
    CHECK_PROXY_CALL_ERR(call_success, error);                       \
    g_assert_true(out_b1);                                           \
  }

#define FOCUS_APP_SYNC(app, time)                                         \
  {                                                                       \
    call_success =                                                        \
        kiran_start_menu_s_call_focus_app_sync(proxy, app, NULL, &error); \
    CHECK_PROXY_CALL_ERR(call_success, error);                            \
    g_print("please waiting %d seconds.\n", sum_sleep_sec);               \
    g_usleep(G_USEC_PER_SEC *(time));                                     \
    sum_sleep_sec -= time;                                                \
  }

void test_frequent_apps(gconstpointer data) {
  GError *error = NULL;
  KiranStartMenuS *proxy = KIRAN_START_MENU_S(data);
  gboolean call_success;
  gboolean out_b1;
  const int kAppMaxLen = 10;
  gchar app[kAppMaxLen];

  int sum_sleep_sec = 12 + 6;

  {
    RESET_FREQUENT_APPS();
    for (int i = 1; i <= 3; ++i) {
      g_snprintf(app, kAppMaxLen, "app%u", i);
      FOCUS_APP_SYNC(app, i + 2);
    }
    FOCUS_APP_SYNC("", 0);

    gchar **top2_apps;
    call_success = kiran_start_menu_s_call_get_nfrequent_apps_sync(
        proxy, 2, &top2_apps, NULL, &error);
    CHECK_PROXY_CALL_ERR(call_success, error);
    int apps_num = g_strv_length(top2_apps);
    g_assert_cmpint(apps_num, ==, 2);
    g_assert_cmpstr(top2_apps[0], ==, "app3");
    g_assert_cmpstr(top2_apps[1], ==, "app2");
  }

  {
    RESET_FREQUENT_APPS();
    for (int i = 1; i <= 3; ++i) {
      g_snprintf(app, kAppMaxLen, "app%u", i);
      FOCUS_APP_SYNC(app, 1);
    }
    FOCUS_APP_SYNC("app4", 3);
    FOCUS_APP_SYNC("", 0);
    gchar **top2_apps;
    call_success = kiran_start_menu_s_call_get_nfrequent_apps_sync(
        proxy, 2, &top2_apps, NULL, &error);
    CHECK_PROXY_CALL_ERR(call_success, error);
    int apps_num = g_strv_length(top2_apps);
    g_assert_cmpint(apps_num, ==, 1);
    g_assert_cmpstr(top2_apps[0], ==, "app4");
  }
}