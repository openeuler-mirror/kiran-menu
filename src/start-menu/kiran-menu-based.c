/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:23:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-08 17:09:34
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-based.c
 */

#include "src/start-menu/kiran-menu-based.h"

#include "src/start-menu/kiran-menu-skeleton.h"

KiranMenuBased *kiran_menu_based_skeleton_new() {
  return g_object_new(KIRAN_TYPE_MENU_SKELETON, NULL);
}

GList *kiran_menu_based_search_app(KiranMenuBased *self, const char *keyword) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_search_app != NULL, NULL);
  return iface->impl_search_app(self, keyword);
}

gboolean kiran_menu_based_add_favorite_app(KiranMenuBased *self,
                                           const char *desktop_id) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), FALSE);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_add_favorite_app != NULL, FALSE);
  return iface->impl_add_favorite_app(self, desktop_id);
}

gboolean kiran_menu_based_del_favorite_app(KiranMenuBased *self,
                                           const char *desktop_id) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), FALSE);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_del_favorite_app != NULL, FALSE);
  return iface->impl_del_favorite_app(self, desktop_id);
}

GList *kiran_menu_based_get_favorite_apps(KiranMenuBased *self) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), FALSE);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_del_favorite_app != NULL, FALSE);
  return iface->impl_del_favorite_app(self, desktop_id);
}

gboolean kiran_menu_based_add_category_app(KiranMenuBased *self,
                                           const char *category,
                                           const char *desktop_id) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), FALSE);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_add_category_app != NULL, FALSE);
  return iface->impl_add_category_app(self, category, desktop_id);
}

gboolean kiran_menu_based_del_category_app(KiranMenuBased *self,
                                           const char *category,
                                           const char *desktop_id) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), FALSE);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_del_category_app != NULL, FALSE);
  return iface->impl_del_category_app(self, category, desktop_id);
}

GList *kiran_menu_based_get_category_apps(KiranMenuBased *self,
                                          const char *category) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_get_favorite_apps != NULL, NULL);
  return iface->impl_get_favorite_apps(self);
}

GHashTable *kiran_menu_based_get_all_category_apps(KiranMenuBased *self) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_get_all_category_apps != NULL, NULL);
  return iface->impl_get_all_category_apps(self);
}

GList *kiran_menu_based_get_nfrequent_apps(KiranMenuBased *self, gint top_n) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self), NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_get_nfrequent_apps != NULL, NULL);
  return iface->impl_get_nfrequent_apps(self, top_n);
}

void kiran_menu_based_reset_frequent_apps(KiranMenuBased *self) {
  KiranMenuBasedInterface *iface;

  g_return_if_fail(KIRAN_IS_MENU_BASED(self));

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_if_fail(iface->impl_reset_frequent_apps != NULL);
  iface->impl_reset_frequent_apps(self);
}

GList *kiran_menu_based_get_nnew_apps(KiranMenuBased *self, gint top_n) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self, NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_get_nnew_apps != NULL, NULL);
  return iface->impl_get_nnew_apps(self, top_n);
}

GList *kiran_menu_based_get_all_sorted_apps(KiranMenuBased *self) {
  KiranMenuBasedInterface *iface;

  g_return_val_if_fail(KIRAN_IS_MENU_BASED(self, NULL);

  iface = KIRAN_MENU_BASED_GET_IFACE(self);
  g_return_val_if_fail(iface->impl_get_all_sorted_apps != NULL, NULL);
  return iface->impl_get_all_sorted_apps(self);
}