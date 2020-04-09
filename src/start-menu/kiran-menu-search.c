/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:01:52
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 00:08:58
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-search.c
 */
#include "src/start-menu/kiran-menu-search.h"

#include "src/start-menu/kiran-app.h"

struct _KiranMenuSearch {
  GObject parent;
};

G_DEFINE_TYPE(KiranMenuSearch, kiran_app_search, G_TYPE_OBJECT)

GList *kiran_menu_search_by_keyword(KiranMenuSearch *self, const char *keyword,
                                    GList *apps) {
  GList *match_apps = NULL;

  for (GList *l = apps; l != NULL; l = l->next) {
    KiranApp *app = l->data;
    const char *comment = kiran_app_get_comment(app);
    const char *locale_comment = kiran_app_get_locale_comment(app);
    const char *name = kiran_app_get_name(app);
    const char *locale_name = kiran_app_get_locale_name(app);

#define STRSTR_KEYWORD(a) (a && g_strrstr(a, keyword) != NULL)

    if (STRSTR_KEYWORD(name) || STRSTR_KEYWORD(locale_name) ||
        STRSTR_KEYWORD(comment) || STRSTR_KEYWORD(locale_comment)) {
      match_apps = g_list_append(match_apps, g_object_ref(app));
    }

#undef STRSTR_KEYWORD
  }
  return match_apps;
}

static void kiran_app_search_init(KiranMenuSearch *self) {}

static void kiran_app_search_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_search_parent_class)->dispose(object);
}

static void kiran_app_search_class_init(KiranMenuSearchClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_app_search_dispose;
}

KiranMenuSearch *kiran_menu_search_get_new() {
  return g_object_new(KIRAN_TYPE_MENU_SEARCH, NULL);
}
