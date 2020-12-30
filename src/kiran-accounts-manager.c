#include "kiran-accounts-manager.h"
#include "log.h"
#include <gio/gio.h>

struct _KiranAccountsManagerPrivate {
    GDBusProxy *dbus_proxy;
    gboolean loaded;
};

#define KIRAN_ACCOUNTS_MANAGER_PRIVATE(o)   kiran_accounts_manager_get_instance_private(o)

G_DEFINE_TYPE_WITH_PRIVATE(KiranAccountsManager, kiran_accounts_manager, G_TYPE_OBJECT)

static void on_dbus_proxy_ready(GObject *source_object, GAsyncResult *result, gpointer userdata)
{
    GError *error = NULL;
    KiranAccountsManager *self = userdata;
    KiranAccountsManagerPrivate *priv = KIRAN_ACCOUNTS_MANAGER_PRIVATE(self);

    priv->dbus_proxy = g_dbus_proxy_new_for_bus_finish(result, &error);
    if (!priv->dbus_proxy) {
        LOG_WARNING("Failed to create dbus proxy for accounts manager: %s", error->message);
        g_error_free(error);
        priv->loaded = FALSE;
        return;
    }

    priv->loaded = TRUE;
}

void kiran_accounts_manager_init(KiranAccountsManager *self)
{
    GError *error = NULL;
    KiranAccountsManagerPrivate *priv = KIRAN_ACCOUNTS_MANAGER_PRIVATE(self);

    priv->dbus_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                     0,
                                                     NULL,
                                                     KIRAN_ACCOUNTS_BUS,
                                                     KIRAN_ACCOUNTS_PATH,
                                                     KIRAN_ACCOUNTS_INTERFACE,
                                                     NULL,
                                                     &error);
    if (error) {
        LOG_WARNING("Failed to create dbus proxy for accounts manager: %s", error->message);
        g_error_free(error);
        priv->loaded = FALSE;
        return;
    }

    priv->loaded = TRUE;
}

void kiran_accounts_manager_finalize(GObject *kobject)
{
    KiranAccountsManager *self = KIRAN_ACCOUNTS_MANAGER(kobject);
    KiranAccountsManagerPrivate *priv = KIRAN_ACCOUNTS_MANAGER_PRIVATE(self);
    if (priv->dbus_proxy)
        g_object_unref(priv->dbus_proxy);

    G_OBJECT_CLASS(kiran_accounts_manager_parent_class)->finalize(kobject);
}

void kiran_accounts_manager_class_init(KiranAccountsManagerClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = kiran_accounts_manager_finalize;
}


KiranAccountsUser *kiran_accounts_manager_get_user_by_id(KiranAccountsManager *self, uid_t uid)
{
    GError *error = NULL;
    GVariant *params, *result, *id_param;
    KiranAccountsUser *user = NULL;
    KiranAccountsManagerPrivate *priv = KIRAN_ACCOUNTS_MANAGER_PRIVATE(self);

    id_param = g_variant_new_uint64(uid);
    params = g_variant_new_tuple(&id_param, 1);
    g_assert (priv->loaded == TRUE);
    result = g_dbus_proxy_call_sync(priv->dbus_proxy,
                                    "FindUserById",
                                    params, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

    if (error)
    {
        LOG_WARNING("Failed to get object path for user %d: %s", uid, error->message),
        g_error_free(error);
    } else {
        const char *object_path;

        g_variant_get(result, "(o)", &object_path);
        user = kiran_accounts_user_new(object_path);
        g_variant_unref(result);
    }
    return user;
}

KiranAccountsManager *kiran_accounts_manager_get_default()
{
    static KiranAccountsManager *manager = NULL;
    if (!manager) {
        manager = g_object_new(KIRAN_ACCOUNTS_TYPE_MANAGER, NULL);
    }

    return manager;
}

