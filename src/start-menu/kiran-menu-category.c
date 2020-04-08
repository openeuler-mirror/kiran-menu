/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 18:08:41
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-category.c
 */
#include "src/start-menu/kiran-menu-category.h"

#include "src/start-menu/kiran-menu-app.h"

typedef struct {
  gint refcount;
  GList *apps;
} CategoryInfo;

struct _KiranMenuCategory {
  GObject parent;
  KiranAppSystem *app_system;
  GHashTable *categorys;
};

G_DEFINE_TYPE(KiranMenuCategory, kiran_menu_category, G_TYPE_OBJECT)

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

static gboolean kiran_menu_category_insert(KiranMenuCategory *self,
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

static gboolean kiran_menu_category_del(KiranMenuCategory *self, char *category,
                                        char *desktop_id) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    return FALSE;
  }
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->app_system, desktop_id);
  return category_info_del_app(category_info, app_info);
}

static gboolean kiran_menu_category_contain(KiranMenuCategory *self,
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

static gboolean handle_get_all_category_apps(KiranStartMenuS *skeleton,
                                             GDBusMethodInvocation *invocation,
                                             KiranMenuCategory *self) {
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

gboolean kiran_menu_category_load(KiranMenuCategory *self, const GList *apps) {
  if (self->categorys) {
    g_hash_table_destroy(self->categorys);
  }
  self->categorys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify)category_info_unref);
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
        kiran_menu_category_insert(self, strv[i], desktop_id);
      }
    }
  }
  g_list_free_full(apps, g_object_unref);
}

gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category,
                                     const KiranMenuApp *menu_app) {
  const gchar *desktop_id = kiran_app_get_desktop_id(KIRAN_APP(menu_app));

  if (kiran_menu_category_contain(self, category, desktop_id)) {
    return FALSE;
  }

  if (kiran_menu_app_add_category(menu_app, category) &&
      kiran_menu_category_insert(self, category, desktop_id)) {
    return TRUE;
  }
  return FALSE;
}

gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category,
                                     const KiranMenuApp *menu_app) {
  const gchar *desktop_id = kiran_app_get_desktop_id(KIRAN_APP(menu_app));
  if (!kiran_menu_category_contain(self, category, desktop_id)) {
    return FALSE;
  }

  if (kiran_menu_category_del(self, category, desktop_id) &&
      kiran_menu_app_del_category(menu_app, category)) {
    return TRUE;
  }
  return FALSE;
}

GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  // g_print("category: %p", category_info);

  gchar **result = category_info_get_desktop_ids(category_info);
}

static void kiran_menu_category_init(KiranMenuCategory *self) {}

static void kiran_menu_category_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_menu_category_parent_class)->dispose(object);
}

static void kiran_menu_category_class_init(KiranMenuCategoryClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_menu_category_dispose;
}

KiranMenuCategory *kiran_menu_category_get_new() {
  return g_object_new(KIRAN_TYPE_MENU_CATEGORY, NULL);
}