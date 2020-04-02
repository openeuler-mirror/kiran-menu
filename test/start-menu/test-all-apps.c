#include "src/start-menu/kiran-app-system.h"
#include "test/start-menu/test-start-menu.h"

#define CASES 10000

void test_all_apps(gconstpointer data) {
  KiranAppSystem *app_system = (KiranAppSystem *)data;
  gint64 start_clock = g_get_real_time();
  for (int i = 0; i <= CASES; ++i) {
    kiran_app_system_get_all_sorted_apps(app_system);
  }
  gint64 end_clock = g_get_real_time();
  gchar **all_sorted_app = kiran_app_system_get_all_sorted_apps(app_system);
  g_print("run %d cases cost %f seconds. sort length: %d\n", CASES,
          (end_clock - start_clock) * 1.0 / G_TIME_SPAN_SECOND,
          g_strv_length(all_sorted_app));
}