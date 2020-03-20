#include "kiran-start-menu-app.h"

#include "kiran-start-menu-bus.h"

struct _KiranStartMenuApp {
  GApplication parent;
  KiranStartMenuBus *start_menu;
};

G_DEFINE_TYPE(KiranStartMenuApp, kiran_start_menu_app, G_TYPE_APPLICATION);

#define INACTIVITY_TIMEOUT 60 * 60 * 1000 /* One minute, in milliseconds */

static gboolean kiran_start_menu_app_dbus_register(GApplication *application,
                                                   GDBusConnection *connection,
                                                   const gchar *object_path,
                                                   GError **error) {
  KiranStartMenuApp *self;

  if (!G_APPLICATION_CLASS(kiran_start_menu_app_parent_class)
           ->dbus_register(application, connection, object_path, error))
    return FALSE;

  self = KIRAN_START_MENU_APP(application);

  return kiran_start_menu_bus_dbus_register(self->start_menu, connection,
                                            object_path, error);
}

static void kiran_start_menu_app_dbus_unregister(GApplication *application,
                                                 GDBusConnection *connection,
                                                 const gchar *object_path) {
  KiranStartMenuApp *self;

  self = KIRAN_START_MENU_APP(application);
  if (self->start_menu)
    kiran_start_menu_bus_dbus_unregister(self->start_menu, connection,
                                         object_path);

  G_APPLICATION_CLASS(kiran_start_menu_app_parent_class)
      ->dbus_unregister(application, connection, object_path);
}

static void kiran_start_menu_app_dispose(GObject *object) {
  KiranStartMenuApp *self;

  self = KIRAN_START_MENU_APP(object);

  g_clear_object(&self->start_menu);

  G_OBJECT_CLASS(kiran_start_menu_app_parent_class)->dispose(object);
}

static void kiran_start_menu_app_init(KiranStartMenuApp *self) {
  self->start_menu = kiran_start_menu_bus_new();

  g_application_set_inactivity_timeout(G_APPLICATION(self), INACTIVITY_TIMEOUT);

  g_application_hold(G_APPLICATION(self));
  g_application_release(G_APPLICATION(self));
}

static void kiran_start_menu_app_startup(GApplication *application) {
  G_APPLICATION_CLASS(kiran_start_menu_app_parent_class)->startup(application);
}

static void kiran_start_menu_app_class_init(KiranStartMenuAppClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

  object_class->dispose = kiran_start_menu_app_dispose;
  app_class->dbus_register = kiran_start_menu_app_dbus_register;
  app_class->dbus_unregister = kiran_start_menu_app_dbus_unregister;
  app_class->startup = kiran_start_menu_app_startup;
}

KiranStartMenuApp *kiran_start_menu_app_get() {
  static KiranStartMenuApp *singleton;

  if (singleton) return singleton;

  singleton = g_object_new(KIRAN_TYPE_START_MENU_APP, "application-id",
                           "com.unikylin.Kiran.StartMenu", "flags",
                           G_APPLICATION_IS_SERVICE, NULL);

  return singleton;
}
