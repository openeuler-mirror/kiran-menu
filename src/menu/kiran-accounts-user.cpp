#include "kiran-accounts-user.h"
#include <gio/gio.h>
#include "lib/base.h"

struct _KiranAccountsUserPrivate
{
    char *object_path;
    GDBusProxy *dbus_proxy;
    gboolean is_loaded;

    char *user_name;
    char *icon_file;
};

G_DEFINE_TYPE_WITH_PRIVATE(KiranAccountsUser, kiran_accounts_user, G_TYPE_OBJECT)

enum
{
    SIGNAL_LOADED = 0,
    SIGNAL_CHANGED,
    SIGNAL_INVALID
};

static guint signals[SIGNAL_INVALID] = {0};

void kiran_accounts_user_init(KiranAccountsUser *self)
{
    KiranAccountsUserPrivate *priv;

    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(self);
    memset(priv, 0, sizeof(KiranAccountsUserPrivate));
}

void kiran_accounts_user_finalize(GObject *kobj)
{
    KiranAccountsUser *self = KIRAN_ACCOUNTS_USER(kobj);
    KiranAccountsUserPrivate *priv;

    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(self);

    g_free(priv->object_path);

    if (priv->dbus_proxy)
        g_object_unref(priv->dbus_proxy);

    G_OBJECT_CLASS(kiran_accounts_user_parent_class)->finalize(kobj);
}

void kiran_accounts_user_class_init(KiranAccountsUserClass *klass)
{
    signals[SIGNAL_LOADED] = g_signal_new("loaded",
                                          G_TYPE_FROM_CLASS(klass),
                                          G_SIGNAL_RUN_FIRST,
                                          0,
                                          NULL,
                                          NULL,
                                          NULL,
                                          G_TYPE_NONE,
                                          0);

    signals[SIGNAL_CHANGED] = g_signal_new("changed",
                                           G_TYPE_FROM_CLASS(klass),
                                           G_SIGNAL_RUN_FIRST,
                                           0,
                                           NULL,
                                           NULL,
                                           NULL,
                                           G_TYPE_NONE,
                                           0);

    G_OBJECT_CLASS(klass)->finalize = kiran_accounts_user_finalize;
}

static void on_dbus_properties_changed(KiranAccountsUser *user, GVariant *changed_properties, GStrv invalidated_properties)
{
    for (int i = 0; i < g_variant_n_children(changed_properties); i++)
    {
        GVariant *child = g_variant_get_child_value(changed_properties, i);

        GVariant *value;
        const char *name;
        g_variant_get(child, "{sv}", &name, &value);
        KLOG_INFO("key '%s' changed", name);
        g_variant_unref(value);
        g_variant_unref(child);
    }

    g_signal_emit_by_name(user, "changed");
}

static void on_dbus_proxy_ready(GObject *source_obj, GAsyncResult *result, gpointer userdata)
{
    KiranAccountsUser *self = (KiranAccountsUser *)userdata;
    KiranAccountsUserPrivate *priv;
    GError *error = NULL;

    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(self);

    priv->dbus_proxy = g_dbus_proxy_new_for_bus_finish(result, &error);
    if (error)
    {
        priv->dbus_proxy = NULL;
        KLOG_WARNING("Failed to connect to dbus: %s", error->message);
        g_error_free(error);
        return;
    }

    priv->is_loaded = TRUE;
    g_signal_emit_by_name(self, "loaded");
    g_signal_connect_swapped(priv->dbus_proxy, "g-properties-changed", G_CALLBACK(on_dbus_properties_changed), self);
}

KiranAccountsUser *kiran_accounts_user_new(const char *object_path)
{
    KiranAccountsUser *user;
    KiranAccountsUserPrivate *priv;

    user = KIRAN_ACCOUNTS_USER(g_object_new(KIRAN_ACCOUNTS_TYPE_USER, NULL));
    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(user);
    priv->object_path = g_strdup(object_path);

    g_dbus_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                             G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                             NULL,
                             KIRAN_ACCOUNTS_BUS,
                             priv->object_path,
                             KIRAN_ACCOUNTS_USER_INTERFACE,
                             NULL,
                             on_dbus_proxy_ready,
                             user);

    return user;
}

static GVariant *kiran_accounts_user_get_property(KiranAccountsUser *user, const char *property_name)
{
    KiranAccountsUserPrivate *priv;

    if (!KIRAN_ACCOUNTS_IS_USER(user))
        return NULL;

    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(user);
    if (!priv->dbus_proxy)
        return NULL;

    return g_dbus_proxy_get_cached_property(priv->dbus_proxy, property_name);
}
static const char *kiran_accounts_user_get_string_property(KiranAccountsUser *user, const char *property_name)
{
    const char *value = NULL;
    GVariant *variant = kiran_accounts_user_get_property(user, property_name);
    if (variant && g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
        value = g_variant_get_string(variant, NULL);

    if (variant)
        g_variant_unref(variant);
    return value;
}

static gboolean kiran_accounts_user_get_boolean_property(KiranAccountsUser *user, const char *property_name)
{
    gboolean value = FALSE;
    GVariant *variant = kiran_accounts_user_get_property(user, property_name);

    if (variant && g_variant_is_of_type(variant, G_VARIANT_TYPE_BOOLEAN))
    {
        value = g_variant_get_boolean(variant);
    }

    if (variant)
        g_variant_unref(variant);
    return value;
}

const char *kiran_accounts_user_get_name(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "user_name");
}
const char *kiran_accounts_user_get_icon_file(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "icon_file");
}

gboolean kiran_accounts_user_get_is_loaded(KiranAccountsUser *user)
{
    KiranAccountsUserPrivate *priv;

    if (!KIRAN_ACCOUNTS_IS_USER(user))
        return FALSE;

    priv = (KiranAccountsUserPrivate *)kiran_accounts_user_get_instance_private(user);
    return priv->is_loaded;
}

guint64 kiran_accounts_user_get_uid(KiranAccountsUser *user)
{
    guint64 uid = -1;
    GVariant *variant = kiran_accounts_user_get_property(user, "uid");

    if (variant && g_variant_is_of_type(variant, G_VARIANT_TYPE_UINT64))
        uid = g_variant_get_uint64(variant);

    if (variant)
        g_variant_unref(variant);
    return uid;
}
const char *kiran_accounts_user_get_shell(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "shell");
}

const char *kiran_accounts_user_get_language(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "language");
}

const char *kiran_accounts_user_get_home_directory(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "home_directory");
}

const char *kiran_accounts_user_get_email(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "email");
}
gboolean kiran_accounts_user_is_locked(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_boolean_property(user, "locked");
}
gboolean kiran_accounts_user_is_system_account(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_boolean_property(user, "system_account");
}
gboolean kiran_accounts_user_is_automatic_login(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_boolean_property(user, "automatic_login");
}

const char *kiran_accounts_user_get_password_hint(KiranAccountsUser *user)
{
    return kiran_accounts_user_get_string_property(user, "password_hint");
}

KiranAccountsPasswordMode kiran_accounts_user_get_password_mode(KiranAccountsUser *user)
{
    guint32 mode;
    GVariant *variant = kiran_accounts_user_get_property(user, "password_mode");

    if (!variant || !g_variant_is_of_type(variant, G_VARIANT_TYPE_UINT32))
        mode = ACCOUNTS_PASSWORD_MODE_LAST;
    else
    {
        mode = g_variant_get_uint32(variant);
    }

    if (variant)
        g_variant_unref(variant);
    return (KiranAccountsPasswordMode)mode;
}
KiranAccountsAccountType kiran_accounts_user_get_account_type(KiranAccountsUser *user)
{
    guint32 account_type;
    GVariant *variant = kiran_accounts_user_get_property(user, "account_type");

    if (!variant || !g_variant_is_of_type(variant, G_VARIANT_TYPE_UINT32))
        account_type = ACCOUNTS_ACCOUNT_TYPE_LAST;
    else
    {
        account_type = g_variant_get_uint32(variant);
    }

    if (variant)
        g_variant_unref(variant);
    return (KiranAccountsAccountType)account_type;
}
