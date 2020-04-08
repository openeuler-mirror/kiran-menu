/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:22:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 15:08:31
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-app.c
 */
#include "src/start-menu/kiran-menu-app.h"

#define DESKTOP_FILE_GROUP "Desktop Entry"

struct _KiranMenuApp {
  KiranApp parent;
};

G_DEFINE_TYPE(KiranMenuApp, kiran_menu_app, KIRAN_TYPE_APP)

gboolean kiran_menu_app_add_category(KiranMenuApp *self, const char *category) {
  const char *categories = kiran_menu_app_get_categories(self);
  const char *file_name = kiran_app_get_file_name(KIRAN_APP(self));
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

gboolean kiran_menu_app_del_category(KiranMenuApp *self, const char *category) {
  const char *file_name = kiran_app_get_file_name(KIRAN_APP(self));
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

static void kiran_menu_app_init(KiranMenuApp *self) {}

static void kiran_menu_app_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_menu_app_parent_class)->dispose(object);
}

static void kiran_menu_app_class_init(KiranMenuAppClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_menu_app_dispose;
}

KiranMenuApp *kiran_app_get_new(const char *desktop_id) {
  return g_object_new(KIRAN_TYPE_MENU_APP, "desktop-id", desktop_id, NULL);
}
