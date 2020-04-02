#include "src/start-menu/kiran-app-category.h"

#include "src/start-menu/kiran-app-info.h"
#include "src/start-menu/kiran-skeleton.h"

typedef struct {
  gint refcount;
  GList *apps;
} CategoryInfo;

struct _KiranAppCategory {
  GObject parent;
  KiranAppSystem *app_system;
  GHashTable *categorys;
};

G_DEFINE_TYPE(KiranAppCategory, kiran_app_category, G_TYPE_OBJECT)

enum {
  CATEGORY_PROP_NONE,
  CATEGORY_PROP_APP_SYSTEM,
};

CategoryInfo *category_info_new(void) {
  CategoryInfo *category;
  category = g_new0(CategoryInfo, 1);
  category->refcount = 1;
  return category;
}

CategoryInfo *category_info_ref(CategoryInfo *category) {
  g_return_val_if_fail(category != NULL, NULL);
  g_return_val_if_fail(category->refcount > 0, NULL);
  category->refcount += 1;
  return category;
}

void category_info_unref(CategoryInfo *category) {
  g_return_if_fail(category != NULL);
  g_return_if_fail(category->refcount > 0);

  category->refcount -= 1;
  if (category->refcount == 0) {
    if (category->apps) g_list_free_full(category->apps, g_object_unref);
    category->apps = NULL;
    g_free(category);
  }
}

gboolean category_info_add_app(CategoryInfo *category, KiranAppInfo *app) {
  g_return_val_if_fail(category != NULL, FALSE);
  g_return_val_if_fail(app != NULL, FALSE);
  if (!category->apps || g_list_find(category->apps, app) == NULL) {
    category->apps = g_list_append(category->apps, app);
    return TRUE;
  }
  return FALSE;
}

gboolean category_info_del_app(CategoryInfo *category, KiranAppInfo *app) {
  g_return_val_if_fail(category != NULL, FALSE);
  g_return_val_if_fail(category->apps != NULL, FALSE);
  g_return_val_if_fail(app != NULL, FALSE);
  if (g_list_find(category->apps, app)) {
    category->apps = g_list_remove(category->apps, app);
    g_object_unref(app);
    return TRUE;
  }
  return FALSE;
}

gboolean category_info_contain_app(CategoryInfo *category, KiranAppInfo *app) {
  g_return_val_if_fail(category != NULL, FALSE);
  g_return_val_if_fail(category->apps != NULL, FALSE);
  return (g_list_find(category->apps, app) == NULL) ? FALSE : TRUE;
}

gchar **category_info_get_desktop_ids(CategoryInfo *category) {
  GPtrArray *desktop_ids = g_ptr_array_new();
  if (category) {
    for (GList *l = category->apps; l != NULL; l = l->next) {
      KiranAppInfo *app = l->data;
      gchar *desktop_id = kiran_app_info_get_desktop_id(app);
      g_ptr_array_add(desktop_ids, g_strdup(desktop_id));
    }
  }
  g_ptr_array_add(desktop_ids, NULL);
  return (gchar **)g_ptr_array_free(desktop_ids, FALSE);
}

static gboolean kiran_app_category_insert(KiranAppCategory *self,
                                          const char *category,
                                          const char *desktop_id) {
  g_return_val_if_fail(category != NULL, FALSE);
  g_return_val_if_fail(desktop_id != NULL, FALSE);

  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    category_info = category_info_new();
    g_hash_table_insert(self->categorys, g_strdup(category), category_info);
  }
  // g_print("c: %s d: %s\n", category, desktop_id);
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->app_system, desktop_id);
  g_return_val_if_fail(app_info != NULL, FALSE);
  return category_info_add_app(category_info, g_object_ref(app_info));
}

static gboolean kiran_app_category_del(KiranAppCategory *self, char *category,
                                       char *desktop_id) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    return FALSE;
  }
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->app_system, desktop_id);
  return category_info_del_app(category_info, app_info);
}

static gboolean kiran_app_category_contain(KiranAppCategory *self,
                                           char *category, char *desktop_id) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    return FALSE;
  }
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->app_system, desktop_id);
  // g_print("contain id: %s app: %d\n", desktop_id, app_info);
  return category_info_contain_app(category_info, app_info);
}

void kiran_app_category_load(KiranAppCategory *self) {
  if (self->categorys) {
    g_hash_table_destroy(self->categorys);
  }
  self->categorys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify)category_info_unref);
  GList *apps = kiran_app_system_get_apps(self->app_system);
  for (GList *l = apps; l != NULL; l = l->next) {
    KiranAppInfo *app_info = l->data;
    const char *desktop_id = kiran_app_info_get_desktop_id(app_info);
    const char *categories = kiran_app_info_get_categories(app_info);
    if (!categories || !desktop_id) {
      continue;
    }
    g_auto(GStrv) strv = g_strsplit(categories, ";", -1);
    gint i = 0;
    for (; strv[i] != NULL; ++i) {
      if (strv[i][0] != '\0') {
        kiran_app_category_insert(self, strv[i], desktop_id);
      }
    }
  }
  g_list_free_full(apps, g_object_unref);
}

static gboolean handle_add_category_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *category, char *desktop_id,
                                        KiranAppCategory *self) {
  gboolean existing = kiran_app_category_contain(self, category, desktop_id);
  gboolean add_success = FALSE;
  if (!existing) {
    KiranAppInfo *app_info =
        kiran_app_system_lookup_app(self->app_system, desktop_id);
    // g_print("add id: %s app: %u\n", desktop_id, app_info);
    if (app_info && kiran_app_info_add_category(app_info, category)) {
      kiran_app_category_insert(self, category, desktop_id);
      add_success = TRUE;
    }
  }

  kiran_start_menu_s_complete_add_category_app(skeleton, invocation,
                                               add_success);
  return TRUE;
}

static gboolean handle_del_category_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *category, char *desktop_id,
                                        KiranAppCategory *self) {
  gboolean existing = kiran_app_category_contain(self, category, desktop_id);
  gboolean del_success = FALSE;
  if (existing) {
    if (kiran_app_category_del(self, category, desktop_id)) {
      KiranAppInfo *app_info =
          kiran_app_system_lookup_app(self->app_system, desktop_id);
      // g_print("del id: %s app: %u\n", desktop_id, app_info);
      if (app_info && kiran_app_info_del_category(app_info, category)) {
        del_success = TRUE;
      }
    }
  }
  kiran_start_menu_s_complete_del_category_app(skeleton, invocation,
                                               del_success);
  return TRUE;
}

static gboolean handle_get_category_apps(KiranStartMenuS *skeleton,
                                         GDBusMethodInvocation *invocation,
                                         char *category,
                                         KiranAppCategory *self) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  // g_print("category: %p", category_info);

  gchar **result = category_info_get_desktop_ids(category_info);
  kiran_start_menu_s_complete_get_category_apps(skeleton, invocation,
                                                (const gchar *const *)result);
  return TRUE;
}

static gboolean handle_get_all_category_apps(KiranStartMenuS *skeleton,
                                             GDBusMethodInvocation *invocation,
                                             KiranAppCategory *self) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
  GHashTableIter iter;
  char *key;
  CategoryInfo *value;

  g_hash_table_iter_init(&iter, self->categorys);
  while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&value)) {
    g_auto(GStrv) apps = category_info_get_desktop_ids(value);
    GVariant *v_apps = g_variant_new_strv((const gchar *const *)apps, -1);
    g_variant_builder_add(&builder, "{sv}", key, v_apps);
  }

  kiran_start_menu_s_complete_get_all_category_apps(
      skeleton, invocation, g_variant_builder_end(&builder));
  return TRUE;
}

static void kiran_app_category_init(KiranAppCategory *self) {}

static void _kiran_app_category_init(KiranAppCategory *self) {
  KiranStartMenuS *skeleton = kiran_start_menu_s_get_default();

  g_signal_connect(skeleton, "handle-add-category-app",
                   G_CALLBACK(handle_add_category_app), self);

  g_signal_connect(skeleton, "handle-del-category-app",
                   G_CALLBACK(handle_del_category_app), self);

  g_signal_connect(skeleton, "handle-get-category-apps",
                   G_CALLBACK(handle_get_category_apps), self);

  g_signal_connect(skeleton, "handle-get-all-category-apps",
                   G_CALLBACK(handle_get_all_category_apps), self);

  g_signal_connect(self->app_system, "installed-changed",
                   G_CALLBACK(kiran_app_category_load), self);

  kiran_app_category_load(self);
}

static void kiran_app_category_get_property(GObject *gobject, guint prop_id,
                                            GValue *value, GParamSpec *pspec) {
  KiranAppCategory *category = KIRAN_APP_CATEGORY(gobject);

  switch (prop_id) {
    case CATEGORY_PROP_APP_SYSTEM:
      if (category->app_system) g_value_set_object(value, category->app_system);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_category_set_property(GObject *gobject, guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  KiranAppCategory *category = KIRAN_APP_CATEGORY(gobject);
  switch (prop_id) {
    case CATEGORY_PROP_APP_SYSTEM:
      category->app_system = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_category_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_category_parent_class)->dispose(object);
}

static void kiran_app_category_class_init(KiranAppCategoryClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = kiran_app_category_get_property;
  object_class->set_property = kiran_app_category_set_property;
  object_class->dispose = kiran_app_category_dispose;

  g_object_class_install_property(
      object_class, CATEGORY_PROP_APP_SYSTEM,
      g_param_spec_object("kiran-app-system", "KiranAppSystem",
                          "KiranAppSystem Object", KIRAN_TYPE_APP_SYSTEM,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

KiranAppCategory *kiran_app_category_get_new(KiranAppSystem *app_system) {
  KiranAppCategory *category = g_object_new(
      KIRAN_TYPE_APP_CATEGORY, "kiran-app-system", app_system, NULL);
  _kiran_app_category_init(category);
  return category;
}