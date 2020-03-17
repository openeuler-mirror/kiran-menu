#include "kiran-start-menu.h"

struct _KiranStartMenu {
  GObject parent;
  KiranStartMenuS *skeleton;
};

G_DEFINE_TYPE(KiranStartMenu, kiran_start_menu, G_TYPE_OBJECT)

static gboolean handle_search_app(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  char *keyword, KiranStartMenu *self) {
  kiran_start_menu_s_complete_search_app(skeleton, invocation, NULL);
  return TRUE;
}

static gboolean handle_add_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranStartMenu *self) {
  kiran_start_menu_s_complete_add_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranStartMenu *self) {
  kiran_start_menu_s_complete_del_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_add_categorical_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranStartMenu *self) {
  kiran_start_menu_s_complete_add_categorical_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_categorical_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranStartMenu *self) {
  kiran_start_menu_s_complete_del_categorical_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_get_categorical_apps(KiranStartMenuS *skeleton,
                                            GDBusMethodInvocation *invocation,
                                            char *category,
                                            char **desktop_files,
                                            KiranStartMenu *self) {
  kiran_start_menu_s_complete_get_categorical_apps(skeleton, invocation, NULL);
  return TRUE;
}

static void kiran_start_menu_init(KiranStartMenu *self) {
  self->skeleton = kiran_start_menu_s_skeleton_new();

  g_signal_connect(self->skeleton, "handle-search-app",
                   G_CALLBACK(handle_search_app), self);

  g_signal_connect(self->skeleton, "handle-add-favorite-app",
                   G_CALLBACK(handle_add_favorite_app), self);

  g_signal_connect(self->skeleton, "handle-del-favorite-app",
                   G_CALLBACK(handle_del_favorite_app), self);

  g_signal_connect(self->skeleton, "handle-add-categorical-app",
                   G_CALLBACK(handle_add_categorical_app), self);

  g_signal_connect(self->skeleton, "handle-del-categorical-app",
                   G_CALLBACK(handle_del_categorical_app), self);

  g_signal_connect(self->skeleton, "handle-get-categorical-apps",
                   G_CALLBACK(handle_get_categorical_apps), self);
}

gboolean kiran_start_menu_dbus_register(KiranStartMenu *self,
                                        GDBusConnection *connection,
                                        const gchar *object_path,
                                        GError **error) {
  GDBusInterfaceSkeleton *skeleton;

  skeleton = G_DBUS_INTERFACE_SKELETON(self->skeleton);

  gboolean ret = g_dbus_interface_skeleton_export(skeleton, connection,
                                                  object_path, error);

  return ret;
}

void kiran_start_menu_dbus_unregister(KiranStartMenu *self,
                                      GDBusConnection *connection,
                                      const gchar *object_path) {
  GDBusInterfaceSkeleton *skeleton;

  skeleton = G_DBUS_INTERFACE_SKELETON(self->skeleton);

  if (g_dbus_interface_skeleton_has_connection(skeleton, connection))
    g_dbus_interface_skeleton_unexport_from_connection(skeleton, connection);
}

static void kiran_start_menu_dispose(GObject *object) {
  KiranStartMenu *self;

  self = KIRAN_START_MENU(object);

  g_clear_object(&self->skeleton);

  G_OBJECT_CLASS(kiran_start_menu_parent_class)->dispose(object);
}

static void kiran_start_menu_class_init(KiranStartMenuClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = kiran_start_menu_dispose;
}

KiranStartMenu *kiran_start_menu_new(void) {
  return g_object_new(KIRAN_TYPE_START_MENU, NULL);
}
