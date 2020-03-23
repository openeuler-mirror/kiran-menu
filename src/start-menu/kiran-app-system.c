#include "src/start-menu/kiran-app-system.h"

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppSystem {
  GObject parent;
  KiranStartMenuS *skeleton;
  // GHashTable *apps_info;
};

G_DEFINE_TYPE(KiranAppSystem, kiran_app_system, G_TYPE_OBJECT)

static gboolean handle_get_all_sorted_apps(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           KiranAppSystem *self) {
  GArray *sort_apps = g_array_new(FALSE, FALSE, sizeof(gchar *));
  GList *registed_apps = g_app_info_get_all();
  for (GList *l = registed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);
    gchar *dup_id = g_strdup(id);
    g_array_append_val(sort_apps, dup_id);
  }

  kiran_start_menu_s_complete_get_all_sorted_apps(
      skeleton, invocation,
      (const gchar *const *)g_array_free(sort_apps, FALSE));
  return TRUE;
}

static gint sort_by_create_time(gconstpointer a, gconstpointer b,
                                gpointer user_data) {
  GHashTable *apps_create_time = user_data;
  guint64 *ca = g_hash_table_lookup(apps_create_time, *(gchar **)a);
  guint64 *cb = g_hash_table_lookup(apps_create_time, *(gchar **)b);
  return *cb - *ca;
}

static gboolean handle_get_nnew_apps(KiranStartMenuS *skeleton,
                                     GDBusMethodInvocation *invocation,
                                     gint arg_top_n, KiranAppSystem *self) {
  GArray *sort_apps = g_array_new(FALSE, FALSE, sizeof(gchar *));
  GHashTable *apps_create_time = g_hash_table_new(g_str_hash, g_str_equal);
  GList *registed_apps = g_app_info_get_all();
  for (GList *l = registed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);
    gchar *dup_id = g_strdup(id);
    g_array_append_val(sort_apps, dup_id);
    GDesktopAppInfo *desktop_app = g_desktop_app_info_new(id);
    if (!desktop_app) continue;
    const char *full_path = g_desktop_app_info_get_filename(desktop_app);
    if (!full_path) continue;
    GFile *file = g_file_new_for_path(full_path);

    GFileInfo *file_info =
        g_file_query_info(file, G_FILE_ATTRIBUTE_TIME_CREATED,
                          G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (file_info)
    {
      guint64 *create_time = g_new(guint64, 1);
      *create_time = g_file_info_get_attribute_uint64(
          file_info, G_FILE_ATTRIBUTE_TIME_CREATED);
      gchar *dup2_id = g_strdup(id);
      g_hash_table_insert(apps_create_time, dup2_id, create_time);
    }

    g_object_unref(file);
    g_object_unref(desktop_app);
  }

  g_array_sort_with_data(sort_apps, sort_by_create_time, apps_create_time);
  if (arg_top_n > sort_apps->len) arg_top_n = sort_apps->len;
  g_array_remove_range(sort_apps, arg_top_n, sort_apps->len - arg_top_n);
  gchar *null = NULL;
  g_array_append_val(sort_apps, null);

  kiran_start_menu_s_complete_get_nnew_apps(
      skeleton, invocation,
      (const gchar *const *)g_array_free(sort_apps, FALSE));
  g_list_free_full(registed_apps, g_object_unref);
  g_hash_table_unref(apps_create_time);
  return TRUE;
}

static void installed_app_change(GAppInfoMonitor *gappinfomonitor,
                                 gpointer user_data) {

}

static void kiran_app_system_init(KiranAppSystem *self) {
  self->skeleton = kiran_start_menu_s_get_default();
  g_signal_connect(self->skeleton, "handle-get-all-sorted-apps",
                   G_CALLBACK(handle_get_all_sorted_apps), self);

  g_signal_connect(self->skeleton, "handle-get-nnew-apps",
                   G_CALLBACK(handle_get_nnew_apps), self);

  GAppInfoMonitor *monitor = g_app_info_monitor_get();
  g_signal_connect(monitor, "changed", G_CALLBACK(installed_app_change), self);
  installed_app_change(monitor, self->skeleton);
}

static void kiran_app_system_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_system_parent_class)->dispose(object);
}

static void kiran_app_system_class_init(KiranAppSystemClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_app_system_dispose;
}

KiranAppSystem *kiran_app_system_get_new() {
  return g_object_new(KIRAN_TYPE_APP_SYSTEM, NULL);
}