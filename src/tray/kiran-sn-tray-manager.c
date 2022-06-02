/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "kiran-sn-tray-manager.h"
#include "kiran-sn-icon.h"
#include "kiran-sn-watcher-gen.h"

#define KIRAN_SN_HOST_BUS_NAME "org.kde.StatusNotifierHost"
#define KIRAN_SN_HOST_OBJECT_PATH "/StatusNotifierHost"
#define KIRAN_SN_ICON_OBJECT_PATH "/StatusNotifierItem"
#define KIRAN_SN_WATCHER_BUS_NAME "org.kde.StatusNotifierWatcher"
#define KIRAN_SN_WATCHER_OBJECT_PATH "/StatusNotifierWatcher"

struct _KiranSnTrayManagerPrivate
{
    gchar *bus_name;
    gchar *object_path;
    guint bus_name_id;
    GCancellable *cancellable;
    guint watcher_id;
    KiranSnWatcherGen *watcher;
    GSList *items;
    gint icon_padding;
    gint icon_size;
};

static void kiran_sn_host_gen_init(KiranSnHostGenIface *iface);
static void kiran_tray_manager_init(KiranTrayManagerInterface *iface);
static void bus_acquired_cb(GDBusConnection *connection,
                            const gchar *name,
                            gpointer user_data);

#define KIRAN_SN_TRAY_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), KIRAN_TYPE_SN_TRAY_MANAGER, KiranSnTrayManagerPrivate))
G_DEFINE_TYPE_WITH_CODE(KiranSnTrayManager, kiran_sn_tray_manager, KIRAN_SN_TYPE_HOST_GEN_SKELETON,
                        G_IMPLEMENT_INTERFACE(KIRAN_SN_TYPE_HOST_GEN, kiran_sn_host_gen_init)
                            G_IMPLEMENT_INTERFACE(KIRAN_TYPE_TRAY_MANAGER, kiran_tray_manager_init))

static void
kiran_sn_host_gen_init(KiranSnHostGenIface *iface)
{
}

static void
kiran_sn_tray_manager_set_icon_size(KiranTrayManager *manager,
                                    gint icon_size)
{
    KiranSnTrayManager *snmanager = KIRAN_SN_TRAY_MANAGER(manager);
    KiranSnTrayManagerPrivate *priv = snmanager->priv;
    ;
    GSList *iterator = NULL;

    priv->icon_size = icon_size;

    for (iterator = priv->items; iterator; iterator = iterator->next)
    {
        kiran_sn_icon_set_icon_size(iterator->data, icon_size);
    }
}

static void
kiran_tray_manager_init(KiranTrayManagerInterface *iface)
{
    iface->set_icon_size = kiran_sn_tray_manager_set_icon_size;
    iface->style_updated = NULL;
}

static void
emit_item_removed_signal(gpointer data,
                         gpointer user_data)
{
    kiran_tray_manager_emit_icon_removed(KIRAN_TRAY_MANAGER(user_data), KIRAN_NOTIFY_ICON(data));
}

static void
kiran_sn_tray_manager_dispose(GObject *object)
{
    KiranSnTrayManager *manager = KIRAN_SN_TRAY_MANAGER(object);
    KiranSnTrayManagerPrivate *priv = manager->priv;
    ;

    if (priv->bus_name_id > 0)
    {
        g_bus_unown_name(priv->bus_name_id);
        priv->bus_name_id = 0;
    }

    if (priv->watcher_id > 0)
    {
        g_bus_unwatch_name(priv->watcher_id);
        priv->watcher_id = 0;
    }

    g_cancellable_cancel(priv->cancellable);
    g_clear_object(&priv->cancellable);

    g_clear_object(&priv->watcher);

    if (priv->items)
    {
        g_slist_foreach(priv->items, emit_item_removed_signal, manager);
        g_slist_free_full(priv->items, g_object_unref);
        priv->items = NULL;
    }

    G_OBJECT_CLASS(kiran_sn_tray_manager_parent_class)->dispose(object);
}

static void
kiran_sn_tray_manager_finalize(GObject *object)
{
    KiranSnTrayManager *manager = KIRAN_SN_TRAY_MANAGER(object);
    KiranSnTrayManagerPrivate *priv = manager->priv;
    ;

    g_clear_pointer(&priv->bus_name, g_free);
    g_clear_pointer(&priv->object_path, g_free);

    G_OBJECT_CLASS(kiran_sn_tray_manager_parent_class)->finalize(object);
}

static void
kiran_sn_tray_manager_class_init(KiranSnTrayManagerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = kiran_sn_tray_manager_dispose;
    gobject_class->finalize = kiran_sn_tray_manager_finalize;

    g_type_class_add_private(gobject_class, sizeof(KiranSnTrayManagerPrivate));
}

static void
kiran_sn_tray_manager_init(KiranSnTrayManager *self)
{
    KiranSnTrayManagerPrivate *priv;
    GBusNameOwnerFlags flags;
    static guint id;

    priv = self->priv = KIRAN_SN_TRAY_MANAGER_GET_PRIVATE(self);

    flags = G_BUS_NAME_OWNER_FLAGS_NONE;
    id++;

    priv->bus_name = g_strdup_printf("%s-%d-%d", KIRAN_SN_HOST_BUS_NAME, getpid(), id);
    priv->object_path = g_strdup_printf("%s/%d", KIRAN_SN_HOST_OBJECT_PATH, id);
    priv->bus_name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                       priv->bus_name,
                                       flags,
                                       bus_acquired_cb,
                                       NULL,
                                       NULL,
                                       self,
                                       NULL);

    priv->icon_size = 16;
    priv->icon_padding = 0;
}

static void
get_bus_name_and_object_path(const gchar *service,
                             gchar **bus_name,
                             gchar **object_path)
{
    gchar *tmp;

    g_assert(*bus_name == NULL);
    g_assert(*object_path == NULL);

    tmp = g_strstr_len(service, -1, "/");

    if (tmp != NULL)
    {
        gchar **strings;

        strings = g_strsplit(service, "/", 2);

        *bus_name = g_strdup(strings[0]);
        *object_path = g_strdup(tmp);

        g_strfreev(strings);
    }
    else
    {
        *bus_name = g_strdup(service);
        *object_path = g_strdup(KIRAN_SN_ICON_OBJECT_PATH);
    }
}

static void
ready_cb(KiranSnIcon *icon,
         KiranSnTrayManager *manager)
{
    kiran_tray_manager_emit_icon_added(KIRAN_TRAY_MANAGER(manager), KIRAN_NOTIFY_ICON(icon));
    gtk_widget_show(GTK_WIDGET(icon));
}

static void
kiran_sn_tray_manager_add_registered_item(KiranSnTrayManager *manager,
                                          const gchar *service)
{
    KiranSnTrayManagerPrivate *priv;
    GtkWidget *icon;
    gchar *bus_name;
    gchar *object_path;

    bus_name = NULL;
    object_path = NULL;

    priv = manager->priv;
    get_bus_name_and_object_path(service, &bus_name, &object_path);

    icon = kiran_sn_icon_new(bus_name, object_path, priv->icon_size);
    g_object_ref_sink(icon);

    priv->items = g_slist_prepend(priv->items, icon);
    g_signal_connect(icon, "ready", G_CALLBACK(ready_cb), manager);

    g_free(bus_name);
    g_free(object_path);
}

static void
item_registered_cb(KiranSnWatcherGen *watcher,
                   const gchar *service,
                   gpointer user_data)
{
    kiran_sn_tray_manager_add_registered_item(KIRAN_SN_TRAY_MANAGER(user_data), service);
}

static void
item_unregistered_cb(KiranSnWatcherGen *watcher,
                     const gchar *service,
                     gpointer user_data)
{
    KiranSnTrayManager *manager = KIRAN_SN_TRAY_MANAGER(user_data);
    KiranSnTrayManagerPrivate *priv = manager->priv;
    ;
    GSList *l;

    for (l = priv->items; l != NULL; l = g_slist_next(l))
    {
        KiranSnIcon *icon;
        gboolean found;
        gchar *bus_name;
        gchar *object_path;

        icon = KIRAN_SN_ICON(l->data);

        found = FALSE;
        bus_name = NULL;
        object_path = NULL;

        get_bus_name_and_object_path(service, &bus_name, &object_path);

        if (g_strcmp0(kiran_sn_icon_get_bus_name(icon), bus_name) == 0 &&
            g_strcmp0(kiran_sn_icon_get_object_path(icon), object_path) == 0)
        {
            priv->items = g_slist_remove(priv->items, icon);
            kiran_tray_manager_emit_icon_removed(KIRAN_TRAY_MANAGER(manager), KIRAN_NOTIFY_ICON(icon));
            g_object_unref(icon);
            found = TRUE;
        }

        g_free(bus_name);
        g_free(object_path);

        if (found)
            break;
    }
}

static void
register_host_cb(GObject *source_object,
                 GAsyncResult *res,
                 gpointer user_data)
{
    KiranSnTrayManager *manager;
    KiranSnTrayManagerPrivate *priv;
    GError *error;
    gchar **items;
    gint i;

    manager = KIRAN_SN_TRAY_MANAGER(user_data);
    priv = manager->priv;

    error = NULL;

    kiran_sn_watcher_gen_call_register_host_finish(KIRAN_SN_WATCHER_GEN(source_object),
                                                   res,
                                                   &error);
    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);

        return;
    }

    g_signal_connect(priv->watcher,
                     "item-registered",
                     G_CALLBACK(item_registered_cb),
                     manager);

    g_signal_connect(priv->watcher,
                     "item-unregistered",
                     G_CALLBACK(item_unregistered_cb),
                     manager);

    items = kiran_sn_watcher_gen_dup_registered_items(priv->watcher);

    if (items)
    {
        for (i = 0; items[i] != NULL; i++)
        {
            kiran_sn_tray_manager_add_registered_item(manager, items[i]);
        }
    }

    g_strfreev(items);
}

static void
proxy_ready_cb(GObject *source_object,
               GAsyncResult *res,
               gpointer user_data)
{
    GError *error;
    KiranSnWatcherGen *watcher;
    KiranSnTrayManager *manager;
    KiranSnTrayManagerPrivate *priv;

    manager = KIRAN_SN_TRAY_MANAGER(user_data);
    priv = manager->priv;

    error = NULL;
    watcher = kiran_sn_watcher_gen_proxy_new_finish(res, &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    priv->watcher = watcher;

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);

        return;
    }

    kiran_sn_watcher_gen_call_register_host(priv->watcher,
                                            priv->object_path,
                                            priv->cancellable,
                                            register_host_cb,
                                            manager);
}

static void
name_vanished_cb(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    KiranSnTrayManager *manager;
    KiranSnTrayManagerPrivate *priv;

    manager = KIRAN_SN_TRAY_MANAGER(user_data);
    priv = manager->priv;

    g_cancellable_cancel(priv->cancellable);
    g_clear_object(&priv->cancellable);

    g_clear_object(&priv->watcher);

    if (priv->items)
    {
        g_slist_foreach(priv->items, emit_item_removed_signal, manager);
        g_slist_free_full(priv->items, g_object_unref);
        priv->items = NULL;
    }
}

static void
name_appeared_cb(GDBusConnection *connection,
                 const gchar *name,
                 const gchar *name_owner,
                 gpointer user_data)
{
    KiranSnTrayManager *manager;
    KiranSnTrayManagerPrivate *priv;

    manager = KIRAN_SN_TRAY_MANAGER(user_data);
    priv = manager->priv;

    g_assert(priv->cancellable == NULL);
    priv->cancellable = g_cancellable_new();

    kiran_sn_watcher_gen_proxy_new(connection,
                                   G_DBUS_PROXY_FLAGS_NONE,
                                   KIRAN_SN_WATCHER_BUS_NAME,
                                   KIRAN_SN_WATCHER_OBJECT_PATH,
                                   priv->cancellable,
                                   proxy_ready_cb,
                                   user_data);
}

static void
bus_acquired_cb(GDBusConnection *connection,
                const char *name,
                gpointer user_data)
{
    KiranSnTrayManager *manager;
    KiranSnTrayManagerPrivate *priv;
    GDBusInterfaceSkeleton *skeleton;
    GError *error;

    manager = KIRAN_SN_TRAY_MANAGER(user_data);
    priv = manager->priv;
    skeleton = G_DBUS_INTERFACE_SKELETON(manager);

    error = NULL;
    g_dbus_interface_skeleton_export(skeleton,
                                     connection,
                                     priv->object_path,
                                     &error);

    if (error != NULL)
    {
        g_warning("%s", error->message);
        g_error_free(error);

        return;
    }

    manager->priv->watcher_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                                 KIRAN_SN_WATCHER_BUS_NAME,
                                                 G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                 name_appeared_cb, name_vanished_cb,
                                                 manager, NULL);
}

KiranTrayManager *
kiran_sn_tray_manager_new(void)
{
    return g_object_new(KIRAN_TYPE_SN_TRAY_MANAGER, NULL);
}
