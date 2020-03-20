#include "src/start-menu/kiran-app-search.h"
#include "src/start-menu/kiran-skeleton.h"


struct _KiranAppSearch {
  GObject parent;
};

G_DEFINE_TYPE(KiranAppSearch, kiran_app_search, G_TYPE_OBJECT)


static gboolean handle_search_app(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  char *keyword, KiranAppSearch *self) {
  GVariant *all_apps = kiran_start_menu_s_get_category_apps(skeleton);
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


static void kiran_app_search_init(KiranAppSearch *self) {
  KiranStartMenuS *skeleton = kiran_start_menu_s_get_default();
  g_signal_connect(skeleton, "handle-search-app",
                   G_CALLBACK(handle_search_app), self);
}


static void kiran_app_search_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_search_parent_class)->dispose(object);
}

static void kiran_app_search_class_init(KiranAppSearchClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_app_search_dispose;
}


KiranAppSearch *kiran_app_search_get_new() {
  return g_object_new(KIRAN_TYPE_APP_SEARCH, NULL);
}
