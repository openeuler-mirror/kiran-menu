#include "kiran-start-menu.h"

struct _KiranStartMenu {
  GObject parent;
  KiranStartMenuS *skeleton;
  GSettings *settings;
};

G_DEFINE_TYPE(KiranStartMenu, kiran_start_menu, G_TYPE_OBJECT)

#define START_MENU_SCHEMA "com.unikylin.Kiran.StartMenu"

static gboolean handle_search_app(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  char *keyword, KiranStartMenu *self) {
  GVariant *all_apps = kiran_start_menu_s_get_all_apps(skeleton);
  GPtrArray *hit_apps = g_ptr_array_new();
  if (all_apps) {
    gsize child_num = g_variant_n_children(all_apps);
    gchar *catergory;
    gchar *desktop_file;
    for (gsize i = 0; i < child_num; ++i) {
      g_variant_get_child(all_apps, i, "(ss)", &desktop_file, &catergory);
      gchar *p = g_strrstr(desktop_file, keyword);
      if (p) {
        g_ptr_array_add(hit_apps, desktop_file);
      }
    }
  }
  g_ptr_array_add(hit_apps, NULL);
  gpointer *result = g_ptr_array_free(hit_apps, FALSE);
  kiran_start_menu_s_complete_search_app(skeleton, invocation,
                                         (const gchar *const *)result);
  return TRUE;
}

static gboolean handle_add_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranStartMenu *self) {
  const gchar *const *apps = kiran_start_menu_s_get_favorite_apps(skeleton);
  if (!g_strv_contains(apps, desktop_file)) {
    GPtrArray *new_apps = g_ptr_array_new();
    GString *t_str;
    for (guint i = 0; apps[i] != NULL; ++i) {
      t_str = g_string_new(apps[i]);
      g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
    }
    t_str = g_string_new(desktop_file);
    g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
    g_ptr_array_add(new_apps, NULL);
    const gchar *const *result =
        (const gchar *const *)g_ptr_array_free(new_apps, FALSE);
    kiran_start_menu_s_set_favorite_apps(skeleton, result);
  }
  kiran_start_menu_s_complete_add_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranStartMenu *self) {
  const gchar *const *apps = kiran_start_menu_s_get_favorite_apps(skeleton);
  if (g_strv_contains(apps, desktop_file)) {
    GPtrArray *new_apps = g_ptr_array_new();
    GString *t_str;
    for (guint i = 0; apps[i] != NULL; ++i) {
      if (!g_str_equal(apps[i], desktop_file)) {
        t_str = g_string_new(apps[i]);
        g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
      }
    }
    g_ptr_array_add(new_apps, NULL);
    const gchar *const *result =
        (const gchar *const *)g_ptr_array_free(new_apps, FALSE);
    kiran_start_menu_s_set_favorite_apps(skeleton, result);
  }
  kiran_start_menu_s_complete_del_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_add_categorical_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranStartMenu *self) {
  GVariant *all_apps = kiran_start_menu_s_get_all_apps(skeleton);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
  gboolean match_app = FALSE;

  if (all_apps) {
    gsize child_num = g_variant_n_children(all_apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_category;
      gchar *elem_desktop_file;
      g_variant_get_child(all_apps, i, "(ss)", &elem_category,
                          &elem_desktop_file);

      if (g_str_equal(desktop_file, elem_desktop_file)) {
        if (g_str_equal(category, elem_category)) {
          match_app = TRUE;
          break;
        }
      } else {
        g_variant_builder_add(&builder, "(ss)", elem_desktop_file,
                              elem_category);
      }
    }
  }
  if (match_app) {
    g_variant_builder_unref(&builder);
  } else {
    g_variant_builder_add(&builder, "(ss)", desktop_file, category);
    kiran_start_menu_s_set_all_apps(skeleton, g_variant_builder_end(&builder));
  }
  kiran_start_menu_s_complete_add_categorical_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_categorical_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranStartMenu *self) {
  GVariant *all_apps = kiran_start_menu_s_get_all_apps(skeleton);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
  gsize child_num = g_variant_n_children(all_apps);

  gboolean match_app = FALSE;
  if (all_apps) {
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_category;
      gchar *elem_desktop_file;
      g_variant_get_child(all_apps, i, "(ss)", &elem_desktop_file,
                          &elem_category);
      if (!match_app && g_str_equal(desktop_file, elem_desktop_file) &&
          g_str_equal(category, elem_category)) {
        match_app = TRUE;
      } else {
        g_variant_builder_add(&builder, "(ss)", elem_desktop_file,
                              elem_category);
      }
    }
  }
  if (match_app) {
    kiran_start_menu_s_set_all_apps(skeleton, g_variant_builder_end(&builder));
  } else {
    g_variant_builder_unref(&builder);
  }

  kiran_start_menu_s_complete_del_categorical_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_get_categorical_apps(KiranStartMenuS *skeleton,
                                            GDBusMethodInvocation *invocation,
                                            char *category,
                                            char **desktop_files,
                                            KiranStartMenu *self) {
  GVariant *all_apps = kiran_start_menu_s_get_all_apps(skeleton);
  GPtrArray *category_apps = g_ptr_array_new();

  if (all_apps) {
    gsize child_num = g_variant_n_children(all_apps);
    gchar *elem_category;
    gchar *elem_desktop_file;

    for (gsize i = 0; i < child_num; ++i) {
      g_variant_get_child(all_apps, i, "(ss)", &elem_desktop_file,
                          &elem_category);
      if (g_str_equal(category, elem_category)) {
        g_ptr_array_add(category_apps, elem_desktop_file);
      }
    }
  }
  g_ptr_array_add(category_apps, NULL);
  gpointer *result = g_ptr_array_free(category_apps, FALSE);
  kiran_start_menu_s_complete_get_categorical_apps(
      skeleton, invocation, (const gchar *const *)result);
  return TRUE;
}

static void installed_app_change(GAppInfoMonitor *gappinfomonitor,
                                 gpointer user_data) {
  KiranStartMenuS *skeleton = KIRAN_START_MENU_S(user_data);
  GVariant *all_apps = kiran_start_menu_s_get_all_apps(skeleton);
  GVariantDict dict;
  g_variant_dict_init(&dict, NULL);

  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));

  if (all_apps) {
    gchar *elem_desktop_file;
    gchar *elem_category;
    GVariantIter iter;
    g_variant_iter_init(&iter, all_apps);
    while (g_variant_iter_loop(&iter, "(ss)", &elem_desktop_file,
                               &elem_category)) {
      g_variant_builder_add(&builder, "(ss)", elem_desktop_file, elem_category);
      g_variant_dict_insert(&dict, elem_desktop_file, "s", elem_category);
    }
  }

  GList *installed_apps = g_app_info_get_all();
  for (GList *l = installed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);
    if (!g_variant_dict_contains(&dict, id)) {
      g_variant_builder_add(&builder, "(ss)", id, "Other");
    }
  }
  kiran_start_menu_s_set_all_apps(skeleton, g_variant_builder_end(&builder));
}

static gboolean all_apps_get_mapping(GValue *value, GVariant *variant,
                                     gpointer user_data) {
  g_value_set_variant(value, variant);
  return TRUE;
}

static GVariant *all_apps_set_mapping(const GValue *value,
                                      const GVariantType *expected_type,
                                      gpointer user_data) {
  return g_value_dup_variant(value);
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

  KiranStartMenuSSkeleton *sskeleton =
      KIRAN_START_MENU_S_SKELETON(self->skeleton);

  self->settings = g_settings_new(START_MENU_SCHEMA);
  g_settings_bind(self->settings, "frequent-apps", sskeleton, "frequent-apps",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "favorite-apps", sskeleton, "favorite-apps",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "new-apps", sskeleton, "new-apps",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind_with_mapping(self->settings, "all-apps", sskeleton,
                               "all-apps", G_SETTINGS_BIND_DEFAULT,
                               all_apps_get_mapping, all_apps_set_mapping, NULL,
                               NULL);

  GAppInfoMonitor *monitor = g_app_info_monitor_get();
  g_signal_connect(monitor, "changed", G_CALLBACK(installed_app_change), self);
  installed_app_change(monitor, self->skeleton);
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
