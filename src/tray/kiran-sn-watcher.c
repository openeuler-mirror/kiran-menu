#include "kiran-sn-watcher.h"

struct _KiranSnWatcherPrivate
{
    guint bus_name_id;
    GSList *hosts;
    GSList *items;
};

typedef enum
{
  KIRAN_WATCH_TYPE_HOST,
  KIRAN_WATCH_TYPE_ITEM
} KiranWatchType;

typedef struct
{
  KiranSnWatcher *watcher;
  KiranWatchType    type;

  gchar         *service;
  gchar         *bus_name;
  gchar         *object_path;
  guint          watch_id;
} KiranWatch;

#define KIRAN_SN_WATCHER_GET_PRIVATE(o)    (G_TYPE_INSTANCE_GET_PRIVATE ((o), KIRAN_TYPE_SN_WATCHER, KiranSnWatcherPrivate))

static void kiran_sn_watcher_gen_init (KiranSnWatcherGenIface *iface);
G_DEFINE_TYPE_WITH_CODE (KiranSnWatcher, kiran_sn_watcher, KIRAN_SN_TYPE_WATCHER_GEN_SKELETON,
                         G_IMPLEMENT_INTERFACE (KIRAN_SN_TYPE_WATCHER_GEN, kiran_sn_watcher_gen_init))

static void
update_registered_items (KiranSnWatcher *watcher)
{
    KiranSnWatcherPrivate *priv;
    GVariantBuilder builder;
    GSList *l;
    GVariant *variant;
    const gchar **items;
  
    g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
    priv = watcher->priv;
  
    for (l = priv->items; l != NULL; l = g_slist_next (l))
    {
        KiranWatch *watch;
        gchar *item;
  
        watch = (KiranWatch *) l->data;
  
        item = g_strdup_printf ("%s%s", watch->bus_name, watch->object_path);
        g_variant_builder_add (&builder, "s", item);
        g_free (item);
    }
  
    variant = g_variant_builder_end (&builder);
    items = g_variant_get_strv (variant, NULL);
  
    kiran_sn_watcher_gen_set_registered_items (KIRAN_SN_WATCHER_GEN (watcher), items);
    g_variant_unref (variant);
    g_free (items);
}

static void
kiran_watch_free (gpointer data)
{
    KiranWatch *watch;
  
    watch = (KiranWatch *) data;
  
    if (watch->watch_id > 0)
        g_bus_unwatch_name (watch->watch_id);
  
    g_free (watch->service);
    g_free (watch->bus_name);
    g_free (watch->object_path);
  
    g_free (watch);
}

static void
name_vanished_cb (GDBusConnection *connection,
                  const char      *name,
                  gpointer         user_data)
{
    KiranWatch *watch;
    KiranSnWatcher *watcher;
    KiranSnWatcherPrivate *priv;
    KiranSnWatcherGen *gen;
  
    watch = (KiranWatch *) user_data;
    watcher = watch->watcher;
    priv = watcher->priv;
    gen = KIRAN_SN_WATCHER_GEN (watcher);
  
    if (watch->type == KIRAN_WATCH_TYPE_HOST)
    {
        priv->hosts = g_slist_remove (priv->hosts, watch);
  
        if (priv->hosts == NULL)
        {
            kiran_sn_watcher_gen_set_is_host_registered (gen, FALSE);
            kiran_sn_watcher_gen_emit_host_registered (gen);
        }
    }
    else if (watch->type == KIRAN_WATCH_TYPE_ITEM)
    {
        gchar *tmp;
  
        priv->items = g_slist_remove (priv->items, watch);
  
        update_registered_items (watcher);
  
        tmp = g_strdup_printf ("%s%s", watch->bus_name, watch->object_path);
        kiran_sn_watcher_gen_emit_item_unregistered (gen, tmp);
        g_free (tmp);
    }
    else
    {
        g_assert_not_reached ();
    }
  
    kiran_watch_free (watch);
}

static KiranWatch *
kiran_watch_new (KiranSnWatcher *watcher,
                 KiranWatchType    type,
                 const gchar   *service,
                 const gchar   *bus_name,
                 const gchar   *object_path)
{
    KiranWatch *watch;
  
    watch = g_new0 (KiranWatch, 1);
  
    watch->watcher = watcher;
    watch->type = type;
  
    watch->service = g_strdup (service);
    watch->bus_name = g_strdup (bus_name);
    watch->object_path = g_strdup (object_path);
    watch->watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION, bus_name,
                                        G_BUS_NAME_WATCHER_FLAGS_NONE, NULL,
                                        name_vanished_cb, watch, NULL);
  
    return watch;
}

static KiranWatch *
kiran_watch_find (GSList      *list,
                  const gchar *bus_name,
                  const gchar *object_path)
{
    GSList *l;
  
    for (l = list; l != NULL; l = g_slist_next (l))
    {
        KiranWatch *watch;
  
        watch = (KiranWatch *) l->data;
  
        if (g_strcmp0 (watch->bus_name, bus_name) == 0 &&
            g_strcmp0 (watch->object_path, object_path) == 0)
	{
         
            return watch;
        }
    }
  
    return NULL;
}

static gboolean
kiran_sn_watcher_handle_register_host (KiranSnWatcherGen     *object,
                                       GDBusMethodInvocation *invocation,
                                       const gchar           *service)
{
    KiranSnWatcher *watcher;
    KiranSnWatcherPrivate *priv;
    const gchar *bus_name;
    const gchar *object_path;
    KiranWatch *watch;
  
    watcher = KIRAN_SN_WATCHER (object);
    priv = watcher->priv;
  
    if (*service == '/')
    {
        bus_name = g_dbus_method_invocation_get_sender (invocation);
        object_path = service;
    }
    else
    {
        bus_name = service;
        object_path = "/StatusNotifierHost";
    }
  
    if (g_dbus_is_name (bus_name) == FALSE)
    {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
                                               G_DBUS_ERROR_INVALID_ARGS,
                                               "D-Bus bus name '%s' is not valid",
                                               bus_name);
  
        return TRUE;
    }
  
    watch = kiran_watch_find (priv->hosts, bus_name, object_path);
  
    if (watch != NULL)
    {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
                                               G_DBUS_ERROR_INVALID_ARGS,
                                               "Status Notifier Host with bus name '%s' and object path '%s' is already registered",
                                               bus_name, object_path);
  
        return TRUE;
    }

    watch = kiran_watch_new (watcher, KIRAN_WATCH_TYPE_HOST, service, bus_name, object_path);
    priv->hosts = g_slist_prepend (priv->hosts, watch);
  
    if (!kiran_sn_watcher_gen_get_is_host_registered (object))
    {
        kiran_sn_watcher_gen_set_is_host_registered (object, TRUE);
        kiran_sn_watcher_gen_emit_host_registered (object);
    }
  
    kiran_sn_watcher_gen_complete_register_host (object, invocation);
  
    return TRUE;
}

static gboolean
kiran_sn_watcher_handle_register_item (KiranSnWatcherGen      *object,
                                       GDBusMethodInvocation *invocation,
                                       const gchar           *service)
{
    KiranSnWatcher *watcher;
    KiranSnWatcherPrivate *priv;
    const gchar *bus_name;
    const gchar *object_path;
    KiranWatch *watch;
    gchar *tmp;
  
    watcher = KIRAN_SN_WATCHER (object);
    priv = watcher->priv;
  
    if (*service == '/')
    {
        bus_name = g_dbus_method_invocation_get_sender (invocation);
        object_path = service;
    }
    else
    {
        bus_name = service;
        object_path = "/StatusNotifierItem";
    }
  
    if (g_dbus_is_name (bus_name) == FALSE)
    {
        g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR,
                                               G_DBUS_ERROR_INVALID_ARGS,
                                               "D-Bus bus name '%s' is not valid",
                                               bus_name);
  
        return TRUE;
    }
  
    watch = kiran_watch_find (priv->items, bus_name, object_path);
  
    if (watch != NULL)
    {
        kiran_sn_watcher_gen_complete_register_item (object, invocation);
  	return TRUE;
    }
    
    watch = kiran_watch_new (watcher, KIRAN_WATCH_TYPE_ITEM, service, bus_name, object_path);
    priv->items = g_slist_prepend (priv->items, watch);
  
    update_registered_items (watcher);
  
    tmp = g_strdup_printf ("%s%s", bus_name, object_path);
    kiran_sn_watcher_gen_emit_item_registered (object, tmp);
    g_free (tmp);
  
    kiran_sn_watcher_gen_complete_register_item (object, invocation);
  
    return TRUE;
}

static void
kiran_sn_watcher_gen_init (KiranSnWatcherGenIface *iface)
{
    iface->handle_register_host = kiran_sn_watcher_handle_register_host;
    iface->handle_register_item = kiran_sn_watcher_handle_register_item;
}

static void
kiran_sn_watcher_dispose (GObject *object)
{
    KiranSnWatcher *watcher;
    KiranSnWatcherPrivate *priv;
  
    watcher = KIRAN_SN_WATCHER (object);
    priv = watcher->priv;
  
    if (priv->bus_name_id > 0)
    {
        g_bus_unown_name (priv->bus_name_id);
        priv->bus_name_id = 0;
    }
  
    if (priv->hosts != NULL)
    {
        g_slist_free_full (priv->hosts, kiran_watch_free);
        priv->hosts = NULL;
    }
  
    if (priv->items != NULL)
    {
        g_slist_free_full (priv->items, kiran_watch_free);
        priv->items = NULL;
    }
  
    G_OBJECT_CLASS (kiran_sn_watcher_parent_class)->dispose (object);
}

static void
bus_acquired_cb (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
    KiranSnWatcher *watcher;
    GDBusInterfaceSkeleton *skeleton;
    GError *error;
  
    watcher = KIRAN_SN_WATCHER (user_data);
    skeleton = G_DBUS_INTERFACE_SKELETON (watcher);
  
    error = NULL;
    g_dbus_interface_skeleton_export (skeleton, connection,
                                      "/StatusNotifierWatcher", &error);
  
    if (error != NULL)
    {
        g_warning ("%s", error->message);
        g_error_free (error);
    }
}

static void
kiran_sn_watcher_class_init (KiranSnWatcherClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = kiran_sn_watcher_dispose;

    g_type_class_add_private (gobject_class, sizeof (KiranSnWatcherPrivate));
}

static void
kiran_sn_watcher_init (KiranSnWatcher *self)
{
    KiranSnWatcherPrivate *priv;
    GBusNameOwnerFlags flags;

    priv = self->priv = KIRAN_SN_WATCHER_GET_PRIVATE (self);

    flags = G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
          G_BUS_NAME_OWNER_FLAGS_REPLACE;

    priv->bus_name_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                                        "org.kde.StatusNotifierWatcher", 
					flags,
                                    	bus_acquired_cb, 
					NULL, NULL, self, NULL);
}

KiranSnWatcher *
kiran_sn_watcher_new (void)
{
    return g_object_new (KIRAN_TYPE_SN_WATCHER, NULL);
}
