#include <gio/gio.h>
#include <glib.h>
#include "src/start-menu/kiran-start-menu-generated.h"

#define DBUS_NAME "com.unikylin.Kiran.StartMenu"
#define DBUS_OBJECT_PATH "/com/unikylin/Kiran/StartMenu"
#define DBUS_OBJECT_MAIN_INTERFACE "com.unikylin.StartMenuS"
#define DBUS_OBJECT_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define CHECK_PROXY_CALL_ERR(result, error)                 \
  {                                                         \
    if (!result) {                                          \
      g_printerr("proxy call error: %s\n", error->message); \
      g_error_free(error);                                  \
      return;                                               \
    }                                                       \
  }

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

static void test_category_apps(gconstpointer data) {
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

static void test_favorite_apps(gconstpointer data) {
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

int main(int argc, char **argv) {
  GError *error = NULL;
  KiranStartMenuS *proxy = kiran_start_menu_s_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, DBUS_NAME, DBUS_OBJECT_PATH,
      NULL, &error);

  if (!proxy) {
    g_printerr("Error creating proxy: %s\n", error->message);
    g_error_free(error);
    return 0;
  }

  g_test_init(&argc, &argv, NULL);
  g_test_add_data_func("/test-start-menu/test-favorite-apps", proxy,
                       test_favorite_apps);

  g_test_add_data_func("/test-start-menu/test-category-apps", proxy,
                       test_category_apps);

  return g_test_run();
}
