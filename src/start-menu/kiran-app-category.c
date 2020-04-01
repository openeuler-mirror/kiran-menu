#include "src/start-menu/kiran-app-category.h"

#include "src/start-menu/kiran-app-info.h"
#include "src/start-menu/kiran-app-system.h"
#include "src/start-menu/kiran-skeleton.h"

typedef struct {
  gint refcount;
  GList *apps;
} CategoryInfo;

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

struct _KiranAppCategory {
  GObject parent;
  KiranAppSystem *system;
  KiranStartMenuS *skeleton;
  GHashTable *categorys;
};

G_DEFINE_TYPE(KiranAppCategory, kiran_app_category, G_TYPE_OBJECT)

static gboolean kiran_app_category_insert(KiranAppCategory *self,
                                          char *category, char *desktop_id) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    category_info = category_info_new();
    g_hash_table_insert(self->categorys, g_strdup(category), category_info);
  }
  // g_print("c: %s d: %s\n", category, desktop_id);
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->system, desktop_id);
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
      kiran_app_system_lookup_app(self->system, desktop_id);
  return category_info_del_app(category_info, app_info);
}

static gboolean kiran_app_category_contain(KiranAppCategory *self,
                                           char *category, char *desktop_id) {
  CategoryInfo *category_info = g_hash_table_lookup(self->categorys, category);
  if (category_info == NULL) {
    return FALSE;
  }
  KiranAppInfo *app_info =
      kiran_app_system_lookup_app(self->system, desktop_id);
  return category_info_contain_app(category_info, app_info);
}

void kiran_app_category_load(KiranAppCategory *self) {
  KiranAppSystemIter iter;
  kiran_app_system_iter_init(&iter);
  gchar *app_id;
  KiranAppInfo *app_info;
  while (kiran_app_system_iter_next(&iter, (gpointer *)&app_id,
                                    (gpointer *)&app_info)) {
    const char *categories = kiran_app_info_get_categories(app_info);
    if (categories == NULL) {
      continue;
    }
    gchar **strv = g_strsplit(categories, ";", -1);
    gint i = 0;
    for (; strv[i] != NULL; ++i) {
      kiran_app_category_insert(self, strv[i], app_id);
    }
    g_strfreev(strv);
  }
}

static gboolean handle_add_category_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *category, char *desktop_id,
                                        KiranAppCategory *self) {
  gboolean existing = kiran_app_category_contain(self, category, desktop_id);
  if (!existing) {
    kiran_app_category_insert(self, category, desktop_id);
  }

  kiran_start_menu_s_complete_add_category_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_category_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *category, char *desktop_id,
                                        KiranAppCategory *self) {
  gboolean existing = kiran_app_category_contain(self, category, desktop_id);
  if (existing) {
    kiran_app_category_del(self, category, desktop_id);
  }
  kiran_start_menu_s_complete_del_category_app(skeleton, invocation, TRUE);
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
  return TRUE;
}

static void kiran_app_category_init(KiranAppCategory *self) {
  self->system = kiran_app_system_get_default();
  self->skeleton = kiran_start_menu_s_get_default();
  self->categorys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify)category_info_unref);

  g_signal_connect(self->skeleton, "handle-add-category-app",
                   G_CALLBACK(handle_add_category_app), self);

  g_signal_connect(self->skeleton, "handle-del-category-app",
                   G_CALLBACK(handle_del_category_app), self);

  g_signal_connect(self->skeleton, "handle-get-category-apps",
                   G_CALLBACK(handle_get_category_apps), self);

  g_signal_connect(self->skeleton, "handle-get-all-category-apps",
                   G_CALLBACK(handle_get_all_category_apps), self);

  kiran_app_category_load(self);
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