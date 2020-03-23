#include "src/start-menu/kiran-start-menu-app.h"

#include <gio/gio.h>
#include "src/start-menu/kiran-app-category.h"
#include "src/start-menu/kiran-app-favorite.h"
#include "src/start-menu/kiran-app-search.h"
#include "src/start-menu/kiran-app-system.h"
#include "src/start-menu/kiran-app-usage.h"
#include "src/start-menu/kiran-skeleton.h"


struct _KiranStartMenuApp {
  GApplication parent;
  KiranStartMenuS *skeleton;
  GSettings *settings;
  KiranAppSystem *system;
  KiranAppUsage *usage;
  KiranAppSearch *search;
  KiranAppFavorite *favorite;
  KiranAppCategory *category;
};

G_DEFINE_TYPE(KiranStartMenuApp, kiran_start_menu_app, G_TYPE_APPLICATION);

#define START_MENU_SCHEMA "com.unikylin.Kiran.StartMenu"
#define INACTIVITY_TIMEOUT 60 * 60 * 1000 /* One minute, in milliseconds */

static gboolean kiran_start_menu_app_dbus_register(GApplication *application,
                                                   GDBusConnection *connection,
                                                   const gchar *object_path,
                                                   GError **error) {
  KiranStartMenuApp *self = KIRAN_START_MENU_APP(application);

  if (!G_APPLICATION_CLASS(kiran_start_menu_app_parent_class)
           ->dbus_register(application, connection, object_path, error)) {
    return FALSE;
  }

  GDBusInterfaceSkeleton *skeleton_iface;
  skeleton_iface = G_DBUS_INTERFACE_SKELETON(self->skeleton);
  return g_dbus_interface_skeleton_export(skeleton_iface, connection,
                                          object_path, error);
}

static void kiran_start_menu_app_dbus_unregister(GApplication *application,
                                                 GDBusConnection *connection,
                                                 const gchar *object_path) {
  KiranStartMenuApp *self = KIRAN_START_MENU_APP(application);
  GDBusInterfaceSkeleton *iface_skeleton;
  iface_skeleton = G_DBUS_INTERFACE_SKELETON(self->skeleton);
  if (g_dbus_interface_skeleton_has_connection(iface_skeleton, connection))
    g_dbus_interface_skeleton_unexport_from_connection(iface_skeleton,
                                                       connection);

  G_APPLICATION_CLASS(kiran_start_menu_app_parent_class)
      ->dbus_unregister(application, connection, object_path);
}

static void kiran_start_menu_app_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_start_menu_app_parent_class)->dispose(object);
}

static gboolean frequent_apps_get_mapping(GValue *value, GVariant *variant,
                                          gpointer user_data) {
  g_value_set_variant(value, variant);
  return TRUE;
}

static GVariant *frequent_apps_set_mapping(const GValue *value,
                                           const GVariantType *expected_type,
                                           gpointer user_data) {
  return g_value_dup_variant(value);
}

static gboolean category_apps_get_mapping(GValue *value, GVariant *variant,
                                     gpointer user_data) {
  g_value_set_variant(value, variant);
  return TRUE;
}

static GVariant *category_apps_set_mapping(const GValue *value,
                                      const GVariantType *expected_type,
                                      gpointer user_data) {
  return g_value_dup_variant(value);
}

static void kiran_start_menu_app_init(KiranStartMenuApp *self) {
  self->skeleton = kiran_start_menu_s_get_default();
  KiranStartMenuSSkeleton *sskeleton =
      KIRAN_START_MENU_S_SKELETON(self->skeleton);

  self->settings = g_settings_new(START_MENU_SCHEMA);
  g_settings_bind_with_mapping(self->settings, "frequent-apps", sskeleton,
                               "frequent-apps", G_SETTINGS_BIND_DEFAULT,
                               frequent_apps_get_mapping,
                               frequent_apps_set_mapping, NULL, NULL);
  g_settings_bind(self->settings, "favorite-apps", sskeleton, "favorite-apps",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind_with_mapping(self->settings, "category-apps", sskeleton,
                               "category-apps", G_SETTINGS_BIND_DEFAULT,
                               category_apps_get_mapping, category_apps_set_mapping, NULL,
                               NULL);

  self->system = kiran_app_system_get_new();
  self->usage = kiran_app_usage_get_new();
  self->search = kiran_app_search_get_new();
  self->favorite = kiran_app_favorite_get_new();
  self->category = kiran_app_category_get_new();

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
