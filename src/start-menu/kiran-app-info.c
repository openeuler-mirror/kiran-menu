#include "src/start-menu/kiran-app-info.h"

#define DESKTOP_FILE_GROUP "Desktop Entry"

struct _KiranAppInfo {
  GObject parent;
  char *app_id;
  GDesktopAppInfo *desktop_app;
  gchar *name;
};

G_DEFINE_TYPE(KiranAppInfo, kiran_app_info, G_TYPE_OBJECT)

enum {
  PROP_NONE,
  PROP_APP_ID,
  PROP_DESKTOP_APP,
  PROP_APP_NAME,
};

GDesktopAppInfo *kiran_app_info_get_desktop_app(KiranAppInfo *app) {
  g_return_val_if_fail(app != NULL, NULL);
  return app->desktop_app;
}

char *kiran_app_info_get_name(KiranAppInfo *app) {
  g_return_val_if_fail(app != NULL, NULL);
  return app->name;
}

char *kiran_app_info_get_desktop_id(KiranAppInfo *app) {
  g_return_val_if_fail(app != NULL, NULL);
  return app->app_id;
}

const char *kiran_app_info_get_categories(KiranAppInfo *app) {
  g_return_val_if_fail(app != NULL, NULL);
  g_return_val_if_fail(app->desktop_app != NULL, NULL);
  return g_desktop_app_info_get_categories(app->desktop_app);
}

gboolean kiran_app_info_add_category(KiranAppInfo *app, const char *category) {
  const char *categories = kiran_app_info_get_categories(app);
  const char *file_name = g_desktop_app_info_get_filename(app->desktop_app);
  g_return_val_if_fail(file_name != NULL, FALSE);
  GString *new_categories = g_string_new(categories);
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  GError *error = NULL;
  gboolean loading = TRUE;
  gboolean saving = TRUE;
  loading = g_key_file_load_from_file(
      key_file, file_name, G_KEY_FILE_NONE | G_KEY_FILE_KEEP_TRANSLATIONS,
      &error);
  if (!loading) {
    g_free(error);
  } else {
    g_string_append(new_categories, category);
    g_string_append_c(new_categories, ';');
    g_key_file_set_string(key_file, DESKTOP_FILE_GROUP, "Categories",
                          new_categories->str);
    saving = g_key_file_save_to_file(key_file, file_name, &error);
    if (!saving) {
      g_free(error);
    }
  }
  g_string_free(new_categories, TRUE);
  return (loading && saving);
}

gboolean kiran_app_info_del_category(KiranAppInfo *app, const char *category) {
  const char *file_name = g_desktop_app_info_get_filename(app->desktop_app);
  g_return_val_if_fail(file_name != NULL, FALSE);
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  GError *error = NULL;
  gboolean loading = TRUE;
  gboolean saving = TRUE;
  loading = g_key_file_load_from_file(
      key_file, file_name,
      G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);
  if (!loading) {
    g_free(error);
  } else {
    gchar **categories = g_key_file_get_string_list(
        key_file, DESKTOP_FILE_GROUP, "Categories", NULL, NULL);
    GString *new_categories = g_string_new(NULL);
    for (gint i = 0; categories[i] != NULL; ++i) {
      if (g_strcmp0(categories[i], category) != 0) {
        new_categories = g_string_append(new_categories, categories[i]);
        new_categories = g_string_append_c(new_categories, ';');
      }
    }
    g_key_file_set_string(key_file, DESKTOP_FILE_GROUP, "Categories",
                          new_categories->str);
    saving = g_key_file_save_to_file(key_file, file_name, &error);
    if (!saving) {
      g_free(error);
    }
    g_strfreev(categories);
    g_string_free(new_categories, TRUE);
  }
  return (loading && saving);
}

static void kiran_app_info_init(KiranAppInfo *self) {}

static void kiran_app_info_get_property(GObject *gobject, guint prop_id,
                                        GValue *value, GParamSpec *pspec) {
  KiranAppInfo *app = KIRAN_APP_INFO(gobject);

  switch (prop_id) {
    case PROP_APP_ID:
      g_value_set_string(value, app->app_id);
      break;
    case PROP_DESKTOP_APP:
      if (app->desktop_app) g_value_set_object(value, app->desktop_app);
      break;
    case PROP_APP_NAME:
      g_value_set_string(value, app->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_set_app_info(KiranAppInfo *app, const char *app_id) {
  app->app_id = g_strdup(app_id);
  app->desktop_app = g_desktop_app_info_new(app_id);
  app->name = g_desktop_app_info_get_string(app->desktop_app, "Name");
}

static void kiran_app_info_set_property(GObject *gobject, guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  KiranAppInfo *app = KIRAN_APP_INFO(gobject);

  switch (prop_id) {
    case PROP_APP_ID:
      kiran_app_set_app_info(app, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_info_dispose(GObject *object) {
  KiranAppInfo *app = KIRAN_APP_INFO(object);
  g_clear_pointer(&app->app_id, g_free);
  g_clear_object(&app->desktop_app);
  g_clear_pointer(&app->name, g_free);

  G_OBJECT_CLASS(kiran_app_info_parent_class)->dispose(object);
}

static void kiran_app_info_class_init(KiranAppInfoClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = kiran_app_info_get_property;
  object_class->set_property = kiran_app_info_set_property;
  object_class->dispose = kiran_app_info_dispose;

  g_object_class_install_property(
      object_class, PROP_APP_ID,
      g_param_spec_string(
          "app-id", "Application id", "The desktop file id", NULL,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_DESKTOP_APP,
      g_param_spec_object("desktop-app", "DesktopAppInfo",
                          "The DesktopAppInfo associated with this app",
                          G_TYPE_DESKTOP_APP_INFO,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_APP_NAME,
      g_param_spec_string("name", "Name", "the desktop file name.", NULL,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}

KiranAppInfo *kiran_app_get_new(const char *app_id) {
  return g_object_new(KIRAN_TYPE_APP_INFO, "app-id", app_id, NULL);
}
