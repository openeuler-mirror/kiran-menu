/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 19:59:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 17:27:46
 * @Description  : 开始菜单类
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-skeleton.c
 */
#include "src/start-menu/kiran-menu-skeleton.h"

#include <gio/gio.h>

#include "src/start-menu/kiran-menu-category.h"
#include "src/start-menu/kiran-menu-favorite.h"
#include "src/start-menu/kiran-menu-search.h"
#include "src/start-menu/kiran-menu-usage.h"

struct _KiranMenuSkeleton {
  GApplication parent;
  KiranStartMenuS *skeleton;
  GSettings *settings;
  KiranAppSystem *system;
  KiranAppUsage *usage;
  KiranAppSearch *search;
  KiranAppFavorite *favorite;
  KiranAppCategory *category;
};

static GList *kiran_menu_skeleton_search_app(KiranMenuBased *self,
                                             const char *keyword) {
  g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

  KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
  GList *apps = kiran_app_system_get_apps(skeleton->system);
  GList match_apps =
      kiran_menu_search_by_keyword(skeleton->search, keyword, apps);
  g_list_free_full(apps, g_object_unref);
  return match_apps;
}

static gboolean kiran_menu_skeleton_add_favorite_app(KiranMenuBased *self,
                                                     const char *desktop_id) {
  g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

  KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
  return kiran_menu_favorite_add_app(skeleton->favorite, desktop_id);
}

static gboolean kiran_menu_skeleton_del_favorite_app(KiranMenuBased *self,
                                                     const char *desktop_id) {
  g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

  KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
  return kiran_menu_favorite_del_app(skeleton->favorite, desktop_id);
}

static GList *kiran_menu_skeleton_get_favorite_apps(KiranMenuBased *self) {
  g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

  KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
  GList *desktop_ids = kiran_menu_favorite_get_favorite_apps(self->favorite);
  GList *favorite_apps = NULL;

  for (GList *l = desktop_ids; l != NULL; l = l->next) {
    gchar *desktop_id = l->data;
    KiranApp *app = kiran_menu_system_lookup_app(skeleton->system, desktop_id);
    if (app) {
      g_list_append(favorite_apps, g_object_ref(app));
    }
  }
  g_list_free_full(desktop_ids, (GDestroyNotify)g_free);
  return favorite_apps;
}

static gboolean kiran_menu_skeleton_add_category_app(KiranMenuBased *self,
                                                     const char *category,
                                                     const char *desktop_id) {}

gboolean kiran_menu_skeleton_del_category_app(KiranMenuBased *self,
                                              const char *category,
                                              const char *desktop_id) {}

GList *kiran_menu_skeleton_get_category_apps(KiranMenuBased *self,
                                             const char *category) {}

GHashTable *kiran_menu_skeleton_get_all_category_apps(KiranMenuBased *self) {}

GList *kiran_menu_skeleton_get_nfrequent_apps(KiranMenuBased *self,
                                              gint top_n) {}

void kiran_menu_skeleton_reset_frequent_apps(KiranMenuBased *self) {}

GList *kiran_menu_skeleton_get_nnew_apps(KiranMenuBased *self, gint top_n) {}

GList *kiran_menu_skeleton_get_all_sorted_apps(KiranMenuBased *self) {}

static void kiran_menu_based_interface_init(KiranMenuBasedInterface *iface) {
  iface->impl_search_app = kiran_menu_skeleton_search_app;

  iface->impl_add_favorite_app = kiran_menu_skeleton_add_favorite_app;
  iface->impl_del_favorite_app = kiran_menu_skeleton_del_favorite_app;
  iface->impl_get_favorite_apps = kiran_menu_skeleton_get_favorite_apps;

  iface->impl_add_category_app = kiran_menu_skeleton_add_category_app;
  iface->impl_del_category_app = kiran_menu_skeleton_del_category_app;
  iface->impl_get_category_apps = kiran_menu_skeleton_get_category_apps;
  iface->impl_get_all_category_apps = kiran_menu_skeleton_get_all_category_apps;

  iface->impl_get_nfrequent_apps = kiran_menu_skeleton_get_nfrequent_apps;
  iface->impl_reset_frequent_apps = kiran_menu_skeleton_reset_frequent_apps;

  iface->impl_get_nnew_apps = kiran_menu_skeleton_get_nnew_apps;

  iface->impl_get_all_sorted_apps = kiran_menu_skeleton_get_all_sorted_apps;
}

G_DEFINE_TYPE(KiranMenuSkeleton, kiran_menu_skeleton, G_OBJECT);
G_DEFINE_TYPE_WITH_CODE(KiranMenuSkeleton, kiran_menu_skeleton,
                        KIRAN_TYPE_MENU_SKELETON,
                        G_IMPLEMENT_INTERFACE(KIRAN_TYPE_MENU_BASED,
                                              kiran_menu_based_interface_init))

#define START_MENU_SCHEMA "com.unikylin.Kiran.StartMenu"

static void kiran_menu_skeleton_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_menu_skeleton_parent_class)->dispose(object);
}

static void kiran_menu_skeleton_init(KiranMenuSkeleton *self) {
  self->skeleton = kiran_start_menu_s_get_default();
  KiranStartMenuSSkeleton *sskeleton =
      KIRAN_START_MENU_S_SKELETON(self->skeleton);

  self->settings = g_settings_new(START_MENU_SCHEMA);
  g_settings_bind_with_mapping(self->settings, "frequent-apps", sskeleton,
                               "frequent-apps", G_SETTINGS_BIND_DEFAULT,
                               frequent_apps_get_mapping,
                               frequent_apps_set_mapping, NULL, NULL);
  g_settings_bind(self->settings, "favorite-apps", sskeleton, "favorite-apps",
                  G_SETTINGS_BIND_DEFAULT);

  self->system = kiran_app_system_get_new();
  self->usage = kiran_app_usage_get_new();
  self->favorite = kiran_app_favorite_get_new();
  self->search = kiran_app_search_get_new(self->system);
  self->category = kiran_app_category_get_new(self->system);
}

static void kiran_menu_skeleton_class_init(KiranMenuSkeletonClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = kiran_menu_skeleton_dispose;
}