#include "src/start-menu/kiran-app-category.h"

#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppCategory {
  GObject parent;
  KiranStartMenuS *skeleton;
  GHashTable *category_apps;
};

G_DEFINE_TYPE(KiranAppCategory, kiran_app_category, G_TYPE_OBJECT)

static void write_category_apps_data(KiranAppCategory *self)
{
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sv)"));
  GHashTableIter iter;
  char *key;
  GVariant *value;

  g_hash_table_iter_init(&iter, self->category_apps);
  while (g_hash_table_iter_next (&iter, (gpointer *)&key, (gpointer *) &value))
  {
    g_variant_builder_add(&builder, "(sv)", key, g_variant_get_variant(value));
  }

  kiran_start_menu_s_set_category_apps(self->skeleton, g_variant_builder_end(&builder));
}

static void read_category_apps_data(KiranAppCategory *self)
{
  GVariant *category_apps = kiran_start_menu_s_get_category_apps(self->skeleton);
  if (category_apps) {
    gsize child_num = g_variant_n_children(category_apps);
    gchar *key;
    GVariant *value;

    for (gsize i = 0; i < child_num; ++i) {
      g_variant_get_child(category_apps, i, "(sv)", &key, &value);
      g_hash_table_insert(self->category_apps, g_strdup(key), (gpointer)g_variant_new_variant(value));
    }
  }
}

static gboolean handle_add_category(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category,
                                           KiranAppCategory *self) {
  if (!g_hash_table_contains(self->category_apps, category))
  {
    const char *null_value[] = {NULL};
    GVariant *vv = g_variant_new_strv(null_value, 0);
    g_hash_table_insert(self->category_apps, g_strdup(category), g_variant_new_variant(vv));
    write_category_apps_data(self);
    kiran_start_menu_s_complete_add_category(skeleton, invocation, TRUE);
  }
  else
  {
    kiran_start_menu_s_complete_add_category(skeleton, invocation, FALSE);
  }
  return TRUE;
}

static gboolean handle_del_category(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category,
                                           KiranAppCategory *self) {
  if (g_hash_table_contains(self->category_apps, category))
  {
    g_hash_table_remove(self->category_apps, category);
    write_category_apps_data(self);
    kiran_start_menu_s_complete_del_category(skeleton, invocation, TRUE);
  }
  else
  {
    kiran_start_menu_s_complete_del_category(skeleton, invocation, FALSE);
  }
  return TRUE;
}

static gboolean handle_add_category_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranAppCategory *self) {
  GVariant *v_apps = g_hash_table_lookup(self->category_apps, category);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
  gboolean match_app = FALSE;

  if (v_apps) {
    GVariant *apps = g_variant_get_variant(v_apps);
    gsize child_num = g_variant_n_children(apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_desktop_file;
      g_variant_get_child(apps, i, "s", &elem_desktop_file);

      if (g_str_equal(desktop_file, elem_desktop_file)) {
        match_app = TRUE;
        break;
      } else {
        g_variant_builder_add(&builder, "s", elem_desktop_file);
      }
    }
  }
  if (match_app) {
    g_variant_builder_unref(&builder);
  } else {
    g_variant_builder_add(&builder, "s", desktop_file);
    GVariant *new_apps = g_variant_builder_end(&builder);
    g_hash_table_insert(self->category_apps, g_strdup(category), g_variant_new_variant(new_apps));
    write_category_apps_data(self);
  }
  kiran_start_menu_s_complete_add_category_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_category_app(KiranStartMenuS *skeleton,
                                           GDBusMethodInvocation *invocation,
                                           char *category, char *desktop_file,
                                           KiranAppCategory *self) {
  GVariant *v_apps = g_hash_table_lookup(self->category_apps, category);
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

  gboolean match_app = FALSE;
  if (v_apps) {
    GVariant *apps = g_variant_get_variant(v_apps);
    gsize child_num = g_variant_n_children(apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *elem_desktop_file;
      g_variant_get_child(apps, i, "s", &elem_desktop_file);
      if (!match_app && g_str_equal(desktop_file, elem_desktop_file)) {
        match_app = TRUE;
      } else {
        g_variant_builder_add(&builder, "s", elem_desktop_file);
      }
    }
  }
  if (match_app) {
    GVariant *new_apps = g_variant_builder_end(&builder);
    g_hash_table_insert(self->category_apps, g_strdup(category), g_variant_new_variant(new_apps));
    write_category_apps_data(self);
  } else {
    g_variant_builder_unref(&builder);
  }
  kiran_start_menu_s_complete_del_category_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_get_category_apps(KiranStartMenuS *skeleton,
                                            GDBusMethodInvocation *invocation,
                                            char *category,
                                            KiranAppCategory *self) {
  GVariant *v_apps = g_hash_table_lookup(self->category_apps, category);
  GPtrArray *new_apps = g_ptr_array_new();

  if (v_apps) {
    GVariant *apps = g_variant_get_variant(v_apps);
    gsize child_num = g_variant_n_children(apps);
    gchar *elem_desktop_file;

    for (gsize i = 0; i < child_num; ++i) {
      g_variant_get_child(apps, i, "s", &elem_desktop_file);
      g_ptr_array_add(new_apps, g_strdup(elem_desktop_file));
    }
  }
  g_ptr_array_add(new_apps, NULL);
  gpointer *result = g_ptr_array_free(new_apps, FALSE);
  kiran_start_menu_s_complete_get_category_apps(
      skeleton, invocation, (const gchar *const *)result);
  return TRUE;
}



static void kiran_app_category_init(KiranAppCategory *self) {
  self->skeleton = kiran_start_menu_s_get_default();
  self->category_apps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

  g_signal_connect(self->skeleton, "handle-add-category",
                   G_CALLBACK(handle_add_category), self);

  g_signal_connect(self->skeleton, "handle-del-category",
                   G_CALLBACK(handle_del_category), self);

  g_signal_connect(self->skeleton, "handle-add-category-app",
                   G_CALLBACK(handle_add_category_app), self);

  g_signal_connect(self->skeleton, "handle-del-category-app",
                   G_CALLBACK(handle_del_category_app), self);

  g_signal_connect(self->skeleton, "handle-get-category-apps",
                   G_CALLBACK(handle_get_category_apps), self);

  read_category_apps_data(self);
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
