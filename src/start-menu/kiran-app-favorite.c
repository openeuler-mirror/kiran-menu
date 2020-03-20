#include "src/start-menu/kiran-app-favorite.h"
#include "src/start-menu/kiran-skeleton.h"


struct _KiranAppFavorite {
  GObject parent;
};

G_DEFINE_TYPE(KiranAppFavorite, kiran_app_favorite, G_TYPE_OBJECT)



static gboolean handle_add_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranAppFavorite *self) {
  const gchar *const *apps = kiran_start_menu_s_get_favorite_apps(skeleton);
  if (!g_strv_contains(apps, desktop_file)) {
    GPtrArray *new_apps = g_ptr_array_new();
    GString *t_str;
    for (guint i = 0; apps[i] != NULL; ++i) {
      t_str = g_string_new(apps[i]);
      g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
    }
    t_str = g_string_new(desktop_file);
    g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
    g_ptr_array_add(new_apps, NULL);
    const gchar *const *result =
        (const gchar *const *)g_ptr_array_free(new_apps, FALSE);
    kiran_start_menu_s_set_favorite_apps(skeleton, result);
  }
  kiran_start_menu_s_complete_add_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}

static gboolean handle_del_favorite_app(KiranStartMenuS *skeleton,
                                        GDBusMethodInvocation *invocation,
                                        char *desktop_file,
                                        KiranAppFavorite *self) {
  const gchar *const *apps = kiran_start_menu_s_get_favorite_apps(skeleton);
  if (g_strv_contains(apps, desktop_file)) {
    GPtrArray *new_apps = g_ptr_array_new();
    GString *t_str;
    for (guint i = 0; apps[i] != NULL; ++i) {
      if (!g_str_equal(apps[i], desktop_file)) {
        t_str = g_string_new(apps[i]);
        g_ptr_array_add(new_apps, g_string_free(t_str, FALSE));
      }
    }
    g_ptr_array_add(new_apps, NULL);
    const gchar *const *result =
        (const gchar *const *)g_ptr_array_free(new_apps, FALSE);
    kiran_start_menu_s_set_favorite_apps(skeleton, result);
  }
  kiran_start_menu_s_complete_del_favorite_app(skeleton, invocation, TRUE);
  return TRUE;
}


static void kiran_app_favorite_init(KiranAppFavorite *self) {
  KiranStartMenuS *skeleton = kiran_start_menu_s_get_default();

  g_signal_connect(skeleton, "handle-add-favorite-app",
                   G_CALLBACK(handle_add_favorite_app), self);

  g_signal_connect(skeleton, "handle-del-favorite-app",
                   G_CALLBACK(handle_del_favorite_app), self);
}


static void kiran_app_favorite_dispose(GObject *object) {
  G_OBJECT_CLASS(kiran_app_favorite_parent_class)->dispose(object);
}

static void kiran_app_favorite_class_init(KiranAppFavoriteClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = kiran_app_favorite_dispose;
}


KiranAppFavorite *kiran_app_favorite_get_new(void) {
  return g_object_new(KIRAN_TYPE_APP_FAVORITE, NULL);
}
