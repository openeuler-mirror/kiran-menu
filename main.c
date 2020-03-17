#include <gio/gio.h>

#include "kiran-start-menu-app.h"

int main(int argc, char **argv) {
  GApplication *app;
  app = G_APPLICATION(kiran_start_menu_app_get());
  return g_application_run(app, argc, argv);
}