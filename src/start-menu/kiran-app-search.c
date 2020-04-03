#include "src/start-menu/kiran-app-search.h"

#include <gio/gdesktopappinfo.h>

#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppSearch {
  GObject parent;
  KiranAppSystem *app_system;
};

G_DEFINE_TYPE(KiranAppSearch, kiran_app_search, G_TYPE_OBJECT)

enum {
  SEARCH_PROP_NONE,
  SEARCH_PROP_APP_SYSTEM,
};

static gboolean handle_search_app(KiranStartMenuS *skeleton,
                                  GDBusMethodInvocation *invocation,
                                  char *keyword, KiranAppSearch *self) {
  GList *apps = kiran_app_system_get_apps(self->app_system);
  GPtrArray *hit_apps = g_ptr_array_new();

  for (GList *l = apps; l != NULL; l = l->next) {
    KiranAppInfo *app = l->data;
    const char *desktop_id = kiran_app_info_get_desktop_id(app);
    const char *categories = kiran_app_info_get_categories(app);
    g_autofree char *name = kiran_app_info_get_name(app);
    g_autofree char *locale_name = kiran_app_info_get_locale_name(app);

    if ((name && g_strrstr(name, keyword) != NULL) ||
        (locale_name && g_strrstr(locale_name, keyword) != NULL) ||
        (categories && g_strrstr(categories, keyword))) {
      g_ptr_array_add(hit_apps, g_strdup(desktop_id));
    }
  }
  g_list_free_full(apps, g_object_unref);

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

static void kiran_app_search_get_property(GObject *gobject, guint prop_id,
                                          GValue *value, GParamSpec *pspec) {
  KiranAppSearch *search = KIRAN_APP_SEARCH(gobject);

  switch (prop_id) {
    case SEARCH_PROP_APP_SYSTEM:
      if (search->app_system) g_value_set_object(value, search->app_system);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_search_set_property(GObject *gobject, guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  KiranAppSearch *search = KIRAN_APP_SEARCH(gobject);

  switch (prop_id) {
    case SEARCH_PROP_APP_SYSTEM:
      search->app_system = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
      break;
  }
}

static void kiran_app_search_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_search_parent_class)->dispose(object);
}

static void kiran_app_search_class_init(KiranAppSearchClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = kiran_app_search_get_property;
  object_class->set_property = kiran_app_search_set_property;
  object_class->dispose = kiran_app_search_dispose;

  g_object_class_install_property(
      object_class, SEARCH_PROP_APP_SYSTEM,
      g_param_spec_object("kiran-app-system", "KiranAppSystem",
                          "KiranAppSystem Object", KIRAN_TYPE_APP_SYSTEM,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

KiranAppSearch *kiran_app_search_get_new(KiranAppSystem *app_system) {
  return g_object_new(KIRAN_TYPE_APP_SEARCH, "kiran-app-system", app_system,
                      NULL);
}
