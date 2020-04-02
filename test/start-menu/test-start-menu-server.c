#include "src/start-menu/kiran-skeleton.h"
#include "src/start-menu/kiran-start-menu-app.h"
#include "test/start-menu/test-start-menu.h"

void test_all_apps(gconstpointer data);

int main(int argc, char **argv) {
  GApplication *app = G_APPLICATION(kiran_start_menu_app_get());
  g_application_register(app, 0, NULL);

  KiranAppSystem *app_system =
      kiran_start_menu_get_app_system(kiran_start_menu_app_get());

  g_test_init(&argc, &argv, NULL);
  g_test_add_data_func("/test-start-menu/test-all-apps", app_system,
                       test_all_apps);

  return g_test_run();
}
