/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 00:55:21
 * @Description  :
 * @FilePath     : /kiran-menu-backend/lib/start-menu/kiran-app.c
 */

#include "lib/kiran-app.h"

#include <gio/gdesktopappinfo.h>

#include "lib/kiran-menu-app.h"

typedef struct _KiranAppPrivate KiranAppPrivate;

struct _KiranAppPrivate
{
    GObject parent;

    gchar *desktop_id;

    gchar *name;
    gchar *locale_name;

    gchar *comment;
    gchar *locale_comment;

    GDesktopAppInfo *desktop_app;
};

G_DEFINE_TYPE_WITH_PRIVATE(KiranApp, kiran_app, G_TYPE_OBJECT);

enum
{
    PROP_NONE,
    PROP_DESKTOP_ID,
};

enum {
    SIGNAL_LAUNCHED = 0,
    SIGANL_LAUNCH_FAILED,
    SIGNAL_INVALID
};

static guint signals[SIGNAL_INVALID];

const gchar *kiran_app_get_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->name;
}

const gchar *kiran_app_get_locale_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->locale_name;
}

const gchar *kiran_app_get_comment(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->comment;
}

const gchar *kiran_app_get_locale_comment(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->locale_comment;
}

const gchar *kiran_app_get_desktop_id(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->desktop_id;
}

const gchar *kiran_app_get_categories(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);
    g_return_val_if_fail(priv->desktop_app != NULL, NULL);

    return g_desktop_app_info_get_categories(priv->desktop_app);
}

const gchar *kiran_app_get_file_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);
    g_return_val_if_fail(priv->desktop_app != NULL, NULL);

    return g_desktop_app_info_get_filename(priv->desktop_app);
}

guint64 kiran_app_get_create_time(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, 0);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    const char *full_path = g_desktop_app_info_get_filename(priv->desktop_app);
    g_return_val_if_fail(full_path != NULL, 0);

    g_autoptr(GFile) file = g_file_new_for_path(full_path);
    g_autoptr(GFileInfo) file_info = g_file_query_info(
        file, G_FILE_ATTRIBUTE_TIME_CREATED, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (file_info)
    {
        return g_file_info_get_attribute_uint64(file_info,
                                                G_FILE_ATTRIBUTE_TIME_CREATED);
    }
    return 0;
}

static void kiran_app_init(KiranApp *self) {}

static void kiran_app_get_property(GObject *gobject, guint prop_id,
                                   GValue *value, GParamSpec *pspec)
{
    KiranApp *app = KIRAN_APP(gobject);

    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    switch (prop_id)
    {
        case PROP_DESKTOP_ID:
            g_value_set_string(value, priv->desktop_id);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static void kiran_app_set_app_info(KiranApp *app, const char *desktop_id)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    priv->desktop_id = g_strdup(desktop_id);
    priv->desktop_app = g_desktop_app_info_new(desktop_id);

#define GET_STRING(key) g_desktop_app_info_get_string(priv->desktop_app, key)
#define GET_LOCALE_STRING(key) \
    g_desktop_app_info_get_locale_string(priv->desktop_app, key)

    priv->name = GET_STRING("Name");
    priv->locale_name = GET_LOCALE_STRING("Name");

    priv->comment = GET_STRING("Comment");
    priv->locale_comment = GET_LOCALE_STRING("Comment");

#undef GET_STRING
#undef GET_LOCALE_STRING
}

static void kiran_app_set_property(GObject *gobject, guint prop_id,
                                   const GValue *value, GParamSpec *pspec)
{
    KiranApp *app = KIRAN_APP(gobject);

    switch (prop_id)
    {
        case PROP_DESKTOP_ID:
            kiran_app_set_app_info(app, g_value_get_string(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
            break;
    }
}

static void kiran_app_dispose(GObject *object)
{
    KiranApp *app = KIRAN_APP(object);

    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    g_clear_pointer(&priv->desktop_id, g_free);
    g_clear_object(&priv->desktop_app);

    g_clear_pointer(&priv->name, g_free);
    g_clear_pointer(&priv->locale_name, g_free);

    g_clear_pointer(&priv->comment, g_free);
    g_clear_pointer(&priv->locale_comment, g_free);

    G_OBJECT_CLASS(kiran_app_parent_class)->dispose(object);
}

static void kiran_app_class_init(KiranAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = kiran_app_get_property;
    object_class->set_property = kiran_app_set_property;
    object_class->dispose = kiran_app_dispose;

    g_object_class_install_property(
        object_class, PROP_DESKTOP_ID,
        g_param_spec_string(
            "desktop-id", "Application id", "The desktop file id", NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

    //signals[SIGNAL_LAUNCHED] = g_signal_new("launched", G_OBJECT_CLASS_TYPE(klass), 0, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

GIcon *kiran_app_get_icon(KiranApp *self)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return g_app_info_get_icon(G_APP_INFO(priv->desktop_app));
}

gboolean kiran_app_launch(KiranApp *self)
{
    GError *error = NULL;
    gboolean res;

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    res = g_app_info_launch(G_APP_INFO(priv->desktop_app), NULL, NULL, NULL);
    return res;
}