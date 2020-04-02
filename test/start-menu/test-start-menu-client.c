#include <glib.h>

#include "src/start-menu/kiran-skeleton.h"
#include "src/start-menu/kiran-start-menu-app.h"
#include "test/start-menu/test-start-menu.h"

void test_favorite_apps(gconstpointer data);
void test_category_apps(gconstpointer data);
void test_frequent_apps(gconstpointer data);

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

  g_test_add_data_func("/test-start-menu/test-frequent-apps", proxy,
                       test_frequent_apps);

  return g_test_run();
}
