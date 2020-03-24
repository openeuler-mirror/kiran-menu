#include "src/start-menu/kiran-app-info.h"

struct _KiranAppInfo {
  GObject parent;
  char *app_id;
  GDesktopAppInfo *desktop_app;
  gint32 create_time;
};

G_DEFINE_TYPE(KiranAppInfo, kiran_app_info, G_TYPE_OBJECT)

enum {
  PROP_NONE,
  PROP_APP_ID,
  PROP_DESKTOP_APP,
};

GDesktopAppInfo *kiran_app_info_get_desktop_app (KiranAppInfo *app)
{
  if (!app)
  {
    return NULL;
  }
  return app->desktop_app;
}

static void kiran_app_info_init(KiranAppInfo *self) {
}


static void kiran_app_info_get_property (GObject    *gobject,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  KiranAppInfo *app = KIRAN_APP_INFO (gobject);

  switch (prop_id)
    {
    case PROP_APP_ID:
      g_value_set_string (value, app->app_id);
      break;
    case PROP_DESKTOP_APP:
      if (app->desktop_app)
        g_value_set_object (value, app->desktop_app);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void kiran_app_set_app_info (KiranAppInfo *app, const char *app_id)
{
  app->app_id = g_strdup(app_id);
  app->desktop_app = g_desktop_app_info_new(app_id);
}

static void kiran_app_info_set_property (GObject      *gobject,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  KiranAppInfo *app = KIRAN_APP_INFO (gobject);

  switch (prop_id)
    {
    case PROP_APP_ID:
      kiran_app_set_app_info (app, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void kiran_app_info_dispose(GObject *object) {
  KiranAppInfo *app = KIRAN_APP_INFO (object);
  g_clear_pointer(&app->app_id, g_free);
  g_clear_object(&app->desktop_app);

  G_OBJECT_CLASS(kiran_app_info_parent_class)->dispose(object);
}

static void kiran_app_info_class_init(KiranAppInfoClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = kiran_app_info_get_property;
  object_class->set_property = kiran_app_info_set_property;
  object_class->dispose = kiran_app_info_dispose;


  g_object_class_install_property (object_class,
                                   PROP_APP_ID,
                                   g_param_spec_string ("app-id",
                                                        "Application id",
                                                        "The desktop file id",
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_DESKTOP_APP,
                                   g_param_spec_object ("desktop-app",
                                                        "DesktopAppInfo",
                                                        "The DesktopAppInfo associated with this app",
                                                        G_TYPE_DESKTOP_APP_INFO,
                                                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

}

KiranAppInfo *kiran_app_get_new(const char *app_id) {
  return g_object_new(KIRAN_TYPE_APP_INFO, "app-id", app_id, NULL);
}
