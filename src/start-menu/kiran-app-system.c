#include "src/start-menu/kiran-app-system.h"
#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppSystem
{
  GObject parent;
  KiranStartMenuS *skeleton;
};

G_DEFINE_TYPE(KiranAppSystem, kiran_app_system, G_TYPE_OBJECT)

static gboolean handle_get_all_sorted_apps(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  KiranAppSystem *self) {

  GArray *sort_apps = g_array_new(FALSE, FALSE, sizeof(gchar*));
  GList *registed_apps = g_app_info_get_all();
  for (GList *l = registed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);
    gchar *dup_id = g_strdup(id);
    g_array_append_val(sort_apps, dup_id);
  }
  
  kiran_start_menu_s_complete_get_all_sorted_apps(skeleton, invocation,
                                         (const gchar *const *)g_array_free(sort_apps, FALSE));
  return TRUE;
}

static void installed_app_change(GAppInfoMonitor *gappinfomonitor,
                                 gpointer user_data) {
  KiranStartMenuS *skeleton = KIRAN_START_MENU_S(user_data);
  kiran_start_menu_s_emit_app_changed(skeleton, 0, "app");
}

static void kiran_app_system_init(KiranAppSystem *self) {
  self->skeleton = kiran_start_menu_s_get_default();
  g_signal_connect(self->skeleton, "handle-get-all-sorted-apps",
                   G_CALLBACK(handle_get_all_sorted_apps), self);

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