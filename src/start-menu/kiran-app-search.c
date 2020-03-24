#include "src/start-menu/kiran-app-search.h"

#include <gio/gdesktopappinfo.h>

#include "src/start-menu/kiran-skeleton.h"
#include "src/start-menu/kiran-app-system.h"

struct _KiranAppSearch {
  GObject parent;
};

G_DEFINE_TYPE(KiranAppSearch, kiran_app_search, G_TYPE_OBJECT)

static gboolean handle_search_app(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  char *keyword, KiranAppSearch *self) {
  KiranAppSystem *system = kiran_app_system_get_default();
  GList *registed_apps = kiran_app_system_get_registered_apps(system);
  GPtrArray *hit_apps = g_ptr_array_new();

  for (GList *l = registed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);

    KiranAppInfo *app = kiran_app_system_lookup_app(system, id);

    const char *category = NULL;
    //const char *name = NULL;
    GDesktopAppInfo *desktop_app = kiran_app_info_get_desktop_app(app);
    if (desktop_app) {
      category = g_desktop_app_info_get_categories(desktop_app);
      //name = g_desktop_app_info_get_string(desktop_app, "Name")
    }


    if (g_strrstr(id, keyword) != NULL ||
        (category && g_strrstr(category, keyword))) {
      g_ptr_array_add(hit_apps, g_strdup(id));
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
  g_signal_connect(skeleton, "handle-search-app", G_CALLBACK(handle_search_app),
                   self);
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
