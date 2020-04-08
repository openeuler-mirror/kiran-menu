/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 15:25:03
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-app.c
 */

#include "src/start-menu/kiran-app.h"

#include <gio/gdesktopappinfo.h>

#include "src/start-menu/kiran-menu-app.h"

struct _KiranApp {
  GObject parent;

  gchar *desktop_id;

  gchar *name;
  gchar *locale_name;

  gchar *comment;
  gchar *locale_comment;

  GDesktopAppInfo *desktop_app;
};

G_DEFINE_TYPE(KiranApp, kiran_app, G_TYPE_OBJECT);

enum {
  PROP_NONE,
  PROP_DESKTOP_ID,
};

const gchar *kiran_app_get_name(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  return self->name;
}

const gchar *kiran_app_get_locale_name(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  return self->locale_name;
}

const gchar *kiran_app_get_comment(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  return self->comment;
}

const gchar *kiran_app_get_locale_comment(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  return self->locale_comment;
}

const gchar *kiran_app_get_desktop_id(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  return self->desktop_id;
}

const gchar *kiran_app_get_categories(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  g_return_val_if_fail(self->desktop_app != NULL, NULL);
  return g_desktop_app_info_get_categories(self->desktop_app);
}

const gchar *kiran_app_get_file_name(KiranApp *self) {
  g_return_val_if_fail(self != NULL, NULL);
  g_return_val_if_fail(self->desktop_app != NULL, NULL);
  return g_desktop_app_info_get_filename(self->desktop_app);
}

static void kiran_app_init(KiranApp *self) {}

static void kiran_app_get_property(GObject *gobject, guint prop_id,
                                   GValue *value, GParamSpec *pspec) {
  KiranApp *app = KIRAN_APP(gobject);

  switch (prop_id) {
    case PROP_DESKTOP_ID:
      g_value_set_string(value, app->desktop_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_set_app_info(KiranApp *app, const char *desktop_id) {
  app->desktop_id = g_strdup(desktop_id);
  app->desktop_app = g_desktop_app_info_new(desktop_id);

#define GET_STRING(key) g_desktop_app_info_get_string(app->desktop_app, key)
#define GET_LOCALE_STRING(key) \
  g_desktop_app_info_get_locale_string(app->desktop_app, key)

  app->name = GET_STRING("Name");
  app->locale_name = GET_LOCALE_STRING("Name");

  app->comment = GET_STRING("Comment");
  app->locale_comment = GET_LOCALE_STRING("Comment");

#undef GET_STRING
#undef GET_LOCALE_STRING
}

static void kiran_app_set_property(GObject *gobject, guint prop_id,
                                   const GValue *value, GParamSpec *pspec) {
  KiranApp *app = KIRAN_APP(gobject);

  switch (prop_id) {
    case PROP_DESKTOP_ID:
      kiran_app_set_app_info(app, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_dispose(GObject *object) {
  KiranApp *app = KIRAN_APP(object);

  g_clear_pointer(&app->desktop_id, g_free);
  g_clear_object(&app->desktop_app);

  g_clear_pointer(&app->name, g_free);
  g_clear_pointer(&app->locale_name, g_free);

  g_clear_pointer(&app->comment, g_free);
  g_clear_pointer(&app->locale_comment, g_free);

  G_OBJECT_CLASS(kiran_app_parent_class)->dispose(object);
}

static void kiran_app_class_init(KiranAppClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = kiran_app_get_property;
  object_class->set_property = kiran_app_set_property;
  object_class->dispose = kiran_app_dispose;

  g_object_class_install_property(
      object_class, PROP_APP_ID,
      g_param_spec_string(
          "desktop-id", "Application id", "The desktop file id", NULL,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}