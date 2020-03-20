#include "src/start-menu/kiran-app-category.h"

#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppCategory {
  GObject parent;
};

G_DEFINE_TYPE(KiranAppCategory, kiran_app_category, G_TYPE_OBJECT)

static gboolean handle_add_category_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranAppCategory *self) {
  GVariant *category_apps = kiran_start_menu_s_get_category_apps(skeleton);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
  gboolean match_app = FALSE;

  if (category_apps) {
    gsize child_num = g_variant_n_children(category_apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_category;
      gchar *elem_desktop_file;
      g_variant_get_child(category_apps, i, "(ss)", &elem_category,
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
    kiran_start_menu_s_set_category_apps(skeleton, g_variant_builder_end(&builder));
  }
  kiran_start_menu_s_complete_add_category_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_category_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranAppCategory *self) {
  GVariant *category_apps = kiran_start_menu_s_get_category_apps(skeleton);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
  gsize child_num = g_variant_n_children(category_apps);

  gboolean match_app = FALSE;
  if (category_apps) {
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_category;
      gchar *elem_desktop_file;
      g_variant_get_child(category_apps, i, "(ss)", &elem_desktop_file,
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
    kiran_start_menu_s_set_category_apps(skeleton, g_variant_builder_end(&builder));
  } else {
    g_variant_builder_unref(&builder);
  }

  kiran_start_menu_s_complete_del_category_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_get_category_apps(KiranStartMenuS *skeleton,
                                            GDBusMethodInvocation *invocation,
                                            char *category,
                                            char **desktop_files,
                                            KiranAppCategory *self) {
  GVariant *category_apps = kiran_start_menu_s_get_category_apps(skeleton);
  GPtrArray *new_category_apps = g_ptr_array_new();

  if (category_apps) {
    gsize child_num = g_variant_n_children(category_apps);
    gchar *elem_category;
    gchar *elem_desktop_file;

    for (gsize i = 0; i < child_num; ++i) {
      g_variant_get_child(category_apps, i, "(ss)", &elem_desktop_file,
                          &elem_category);
      if (g_str_equal(category, elem_category)) {
        g_ptr_array_add(new_category_apps, elem_desktop_file);
      }
    }
  }
  g_ptr_array_add(new_category_apps, NULL);
  gpointer *result = g_ptr_array_free(new_category_apps, FALSE);
  kiran_start_menu_s_complete_get_category_apps(
      skeleton, invocation, (const gchar *const *)result);
  return TRUE;
}

static void kiran_app_category_init(KiranAppCategory *self) {
  KiranStartMenuS *skeleton = kiran_start_menu_s_get_default();

  g_signal_connect(skeleton, "handle-add-category-app",
                   G_CALLBACK(handle_add_category_app), self);

  g_signal_connect(skeleton, "handle-del-category-app",
                   G_CALLBACK(handle_del_category_app), self);

  g_signal_connect(skeleton, "handle-get-category-apps",
                   G_CALLBACK(handle_get_category_apps), self);
}

static void kiran_app_category_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_category_parent_class)->dispose(object);
}

static void kiran_app_category_class_init(KiranAppCategoryClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_app_category_dispose;
}

KiranAppCategory *kiran_app_category_get_new(void) {
  return g_object_new(KIRAN_TYPE_APP_CATEGORY, NULL);
}
