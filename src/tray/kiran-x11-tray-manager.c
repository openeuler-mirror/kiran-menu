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

#include "kiran-x11-tray-manager.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include "kiran-x11-tray-icon.h"
#include "kiran-x11-tray-socket.h"

struct _KiranX11TrayManagerPrivate
{
    GtkWidget *invisible;
    GHashTable *tray_icon_table;
    GdkScreen *screen;
    GtkOrientation orientation;
    gint padding;
    gint icon_size;
    GdkRGBA fg;
    GdkRGBA error;
    GdkRGBA warning;
    GdkRGBA success;
    GdkAtom selection_atom;
};

enum
{
    PROP_0,
    PROP_ORIENTATION,
    PROP_ICON_PADDING,
    PROP_ICON_SIZE,
    PROP_SCREEN
};

#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

static void kiran_tray_manager_init(KiranTrayManagerInterface *iface);
static GdkFilterReturn kiran_x11_tray_manager_window_filter(GdkEvent *xev,
                                                            GdkEvent *event,
                                                            gpointer data);
static void kiran_x11_tray_manager_handle_dock_request(KiranX11TrayManager *manager,
                                                       XClientMessageEvent *xevent);
static gboolean kiran_x11_tray_manager_plug_removed(GtkSocket *socket,
                                                    KiranX11TrayManager *manager);
static void kiran_x11_manager_set_icon_size_property(KiranX11TrayManager *manager);

#define KIRAN_X11_TRAY_MANAGER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), KIRAN_TYPE_X11_TRAY_MANAGER, KiranX11TrayManagerPrivate))
G_DEFINE_TYPE_WITH_CODE(KiranX11TrayManager, kiran_x11_tray_manager, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(KIRAN_TYPE_TRAY_MANAGER, kiran_tray_manager_init))
static void
kiran_x11_tray_manager_set_icon_size(KiranTrayManager *manager,
                                     gint icon_size)
{
    KiranX11TrayManager *xmanager = KIRAN_X11_TRAY_MANAGER(manager);
    KiranX11TrayManagerPrivate *priv = xmanager->priv;

    priv->icon_size = icon_size;

    kiran_x11_manager_set_icon_size_property(xmanager);
}

static void
kiran_tray_manager_init(KiranTrayManagerInterface *iface)
{
    iface->set_icon_size = kiran_x11_tray_manager_set_icon_size;
}

static void
kiran_x11_tray_manager_unmanage(KiranX11TrayManager *manager)
{
    GdkDisplay *display;
    guint32 timestamp;
    GtkWidget *invisible;
    GdkWindow *window;
    KiranX11TrayManagerPrivate *priv = manager->priv;

    if (priv->invisible == NULL)
        return;

    invisible = priv->invisible;
    window = gtk_widget_get_window(invisible);

    g_assert(GTK_IS_INVISIBLE(invisible));
    g_assert(gtk_widget_get_realized(invisible));
    g_assert(GDK_IS_WINDOW(window));

    display = gtk_widget_get_display(invisible);

    if (gdk_selection_owner_get_for_display(display, priv->selection_atom) ==
        window)
    {
        timestamp = gdk_x11_get_server_time(window);
        gdk_selection_owner_set_for_display(display,
                                            NULL,
                                            priv->selection_atom,
                                            timestamp,
                                            TRUE);
    }

    gdk_window_remove_filter(window,
                             (GdkFilterFunc)kiran_x11_tray_manager_window_filter, manager);

    priv->invisible = NULL; /* prior to destroy for reentrancy paranoia */
    gtk_widget_destroy(invisible);
}

static void
socket_plug_removed_signal_disconnect(gpointer key,
                                      gpointer value,
                                      gpointer user_data)
{
    KiranX11TrayManager *manager = KIRAN_X11_TRAY_MANAGER(user_data);
    KiranX11TrayIcon *icon = KIRAN_X11_TRAY_ICON(value);
    GtkWidget *socket = kiran_x11_tray_icon_get_socket(icon);

    //disconnect signal
    g_signal_handlers_disconnect_by_func(socket,
                                         G_CALLBACK(kiran_x11_tray_manager_plug_removed),
                                         manager);
}

static void
kiran_x11_tray_manager_finalize(GObject *object)
{
    KiranX11TrayManager *manager = KIRAN_X11_TRAY_MANAGER(object);
    KiranX11TrayManagerPrivate *priv = manager->priv;

    kiran_x11_tray_manager_unmanage(manager);
    g_hash_table_foreach(priv->tray_icon_table, socket_plug_removed_signal_disconnect, manager);
    g_hash_table_destroy(priv->tray_icon_table);

    G_OBJECT_CLASS(kiran_x11_tray_manager_parent_class)->finalize(object);
}

static void
kiran_x11_tray_manager_class_init(KiranX11TrayManagerClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->finalize = kiran_x11_tray_manager_finalize;
    g_type_class_add_private(gobject_class, sizeof(KiranX11TrayManagerPrivate));
}

static void
kiran_x11_tray_manager_init(KiranX11TrayManager *self)
{
    KiranX11TrayManagerPrivate *priv;

    priv = self->priv = KIRAN_X11_TRAY_MANAGER_GET_PRIVATE(self);

    priv->tray_icon_table = g_hash_table_new(NULL, NULL);

    priv->padding = 4;
    priv->icon_size = 16;

    priv->fg.red = 0.0;
    priv->fg.green = 0.0;
    priv->fg.blue = 0.0;
    priv->fg.alpha = 1.0;

    priv->error.red = 1.0;
    priv->error.green = 0.0;
    priv->error.blue = 0.0;
    priv->error.alpha = 1.0;

    priv->warning.red = 1.0;
    priv->warning.green = 1.0;
    priv->warning.blue = 0.0;
    priv->warning.alpha = 1.0;

    priv->success.red = 0.0;
    priv->success.green = 1.0;
    priv->success.blue = 0.0;
    priv->success.alpha = 1.0;
}

static GdkFilterReturn
kiran_x11_tray_manager_window_filter(GdkEvent *xev,
                                     GdkEvent *event,
                                     gpointer data)
{
    XEvent *xevent = (GdkXEvent *)xev;
    KiranX11TrayManager *manager = KIRAN_X11_TRAY_MANAGER(data);

    if (xevent->type == ClientMessage)
    {
        if (xevent->xclient.data.l[1] == SYSTEM_TRAY_REQUEST_DOCK)
        {
            kiran_x11_tray_manager_handle_dock_request(manager,
                                                       (XClientMessageEvent *)xevent);
            return GDK_FILTER_REMOVE;
        }
    }
    else if (xevent->type == SelectionClear)
    {
        kiran_x11_tray_manager_unmanage(manager);
    }

    return GDK_FILTER_CONTINUE;
}

static gboolean
kiran_x11_tray_manager_plug_removed(GtkSocket *socket,
                                    KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(socket));
    KiranX11TrayIcon *icon = KIRAN_X11_TRAY_ICON(parent);
    Window icon_window = kiran_x11_tray_icon_get_icon_window(icon);

    g_hash_table_remove(priv->tray_icon_table, GINT_TO_POINTER(icon_window));
    kiran_tray_manager_emit_icon_removed(KIRAN_TRAY_MANAGER(manager), KIRAN_NOTIFY_ICON(parent));

    return FALSE;
}

static void
kiran_x11_tray_manager_handle_dock_request(KiranX11TrayManager *manager,
                                           XClientMessageEvent *xevent)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    Window icon_window = xevent->data.l[2];
    GtkWidget *icon;
    GtkWidget *socket;

    if (g_hash_table_lookup(priv->tray_icon_table, GINT_TO_POINTER(icon_window)))
    {
        return;
    }

    icon = kiran_x11_tray_icon_new(icon_window);
    if (!icon)
        return;

    socket = kiran_x11_tray_icon_get_socket(KIRAN_X11_TRAY_ICON(icon));

    kiran_tray_manager_emit_icon_added(KIRAN_TRAY_MANAGER(manager), KIRAN_NOTIFY_ICON(icon));

    if (!GTK_IS_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(icon))))
    {
        gtk_widget_destroy(icon);
        return;
    }

    kiran_x11_tray_socket_add_id(KIRAN_X11_TRAY_SOCKET(socket), icon_window, priv->screen);

    g_signal_connect(socket, "plug-removed",
                     G_CALLBACK(kiran_x11_tray_manager_plug_removed), manager);

    if (!gtk_socket_get_plug_window(GTK_SOCKET(socket)))
    {
        kiran_tray_manager_emit_icon_removed(KIRAN_TRAY_MANAGER(manager), KIRAN_NOTIFY_ICON(icon));
        return;
    }

    g_hash_table_insert(priv->tray_icon_table,
                        GINT_TO_POINTER(icon_window), icon);
    gtk_widget_show(icon);
}

static void
kiran_x11_manager_set_orientation_property(KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GdkWindow *window;
    GdkDisplay *display;
    Atom orientation_atom;
    gulong data[1];

    g_return_if_fail(priv->invisible != NULL);
    window = gtk_widget_get_window(priv->invisible);
    g_return_if_fail(window != NULL);

    display = gtk_widget_get_display(priv->invisible);
    orientation_atom = gdk_x11_get_xatom_by_name_for_display(display,
                                                             "_NET_SYSTEM_TRAY_ORIENTATION");

    data[0] = priv->orientation == GTK_ORIENTATION_HORIZONTAL ? SYSTEM_TRAY_ORIENTATION_HORZ : SYSTEM_TRAY_ORIENTATION_VERT;

    XChangeProperty(GDK_DISPLAY_XDISPLAY(display),
                    GDK_WINDOW_XID(window),
                    orientation_atom,
                    XA_CARDINAL, 32,
                    PropModeReplace,
                    (guchar *)&data, 1);
}

static void
kiran_x11_manager_set_visual_property(KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;

    GdkWindow *window;
    GdkDisplay *display;
    Visual *xvisual;
    Atom visual_atom;
    gulong data[1];

    g_return_if_fail(priv->invisible != NULL);
    window = gtk_widget_get_window(priv->invisible);
    g_return_if_fail(window != NULL);

    display = gtk_widget_get_display(priv->invisible);
    visual_atom = gdk_x11_get_xatom_by_name_for_display(display,
                                                        "_NET_SYSTEM_TRAY_VISUAL");

    if (gdk_screen_get_rgba_visual(priv->screen) != NULL)
    {
        xvisual = GDK_VISUAL_XVISUAL(gdk_screen_get_rgba_visual(priv->screen));
    }
    else
    {
        xvisual = GDK_VISUAL_XVISUAL(gdk_screen_get_system_visual(priv->screen));
    }

    data[0] = XVisualIDFromVisual(xvisual);

    XChangeProperty(GDK_DISPLAY_XDISPLAY(display),
                    GDK_WINDOW_XID(window),
                    visual_atom,
                    XA_VISUALID, 32,
                    PropModeReplace,
                    (guchar *)&data, 1);
}

static void
kiran_x11_manager_set_padding_property(KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GdkWindow *window;
    GdkDisplay *display;
    Atom atom;
    gulong data[1];

    g_return_if_fail(priv->invisible != NULL);
    window = gtk_widget_get_window(priv->invisible);
    g_return_if_fail(window != NULL);

    display = gtk_widget_get_display(priv->invisible);
    atom = gdk_x11_get_xatom_by_name_for_display(display,
                                                 "_NET_SYSTEM_TRAY_PADDING");

    data[0] = priv->padding;

    XChangeProperty(GDK_DISPLAY_XDISPLAY(display),
                    GDK_WINDOW_XID(window),
                    atom,
                    XA_CARDINAL, 32,
                    PropModeReplace,
                    (guchar *)&data, 1);
}

static void
kiran_x11_manager_set_icon_size_property(KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GdkWindow *window;
    GdkDisplay *display;
    Atom atom;
    gulong data[1];

    g_return_if_fail(priv->invisible != NULL);
    window = gtk_widget_get_window(priv->invisible);
    g_return_if_fail(window != NULL);

    display = gtk_widget_get_display(priv->invisible);
    atom = gdk_x11_get_xatom_by_name_for_display(display,
                                                 "_NET_SYSTEM_TRAY_ICON_SIZE");

    data[0] = priv->icon_size;

    XChangeProperty(GDK_DISPLAY_XDISPLAY(display),
                    GDK_WINDOW_XID(window),
                    atom,
                    XA_CARDINAL, 32,
                    PropModeReplace,
                    (guchar *)&data, 1);
}

static void
kiran_x11_manager_set_colors_property(KiranX11TrayManager *manager)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GdkWindow *window;
    GdkDisplay *display;
    Atom atom;
    gulong data[12];

    g_return_if_fail(priv->invisible != NULL);
    window = gtk_widget_get_window(priv->invisible);
    g_return_if_fail(window != NULL);

    display = gtk_widget_get_display(priv->invisible);
    atom = gdk_x11_get_xatom_by_name_for_display(display,
                                                 "_NET_SYSTEM_TRAY_COLORS");

    data[0] = priv->fg.red * 65535;
    data[1] = priv->fg.green * 65535;
    data[2] = priv->fg.blue * 65535;
    data[3] = priv->error.red * 65535;
    data[4] = priv->error.green * 65535;
    data[5] = priv->error.blue * 65535;
    data[6] = priv->warning.red * 65535;
    data[7] = priv->warning.green * 65535;
    data[8] = priv->warning.blue * 65535;
    data[9] = priv->success.red * 65535;
    data[10] = priv->success.green * 65535;
    data[11] = priv->success.blue * 65535;

    XChangeProperty(GDK_DISPLAY_XDISPLAY(display),
                    GDK_WINDOW_XID(window),
                    atom,
                    XA_CARDINAL, 32,
                    PropModeReplace,
                    (guchar *)&data, 12);
}

gboolean
kiran_x11_tray_manager_manage_screen(KiranX11TrayManager *manager,
                                     GdkScreen *screen)
{
    KiranX11TrayManagerPrivate *priv = manager->priv;
    GdkDisplay *display;
    GdkWindow *window;
    Screen *xscreen;
    gchar *selection_atom_name;
    guint32 timestamp;

    display = gdk_screen_get_display(screen);
    xscreen = GDK_SCREEN_XSCREEN(screen);

    priv->invisible = gtk_invisible_new_for_screen(screen);
    gtk_widget_realize(priv->invisible);

    gtk_widget_add_events(priv->invisible,
                          GDK_PROPERTY_CHANGE_MASK | GDK_STRUCTURE_MASK);

    selection_atom_name = g_strdup_printf("_NET_SYSTEM_TRAY_S%d",
                                          gdk_x11_screen_get_screen_number(screen));
    priv->selection_atom = gdk_atom_intern(selection_atom_name, FALSE);
    g_free(selection_atom_name);

    window = gtk_widget_get_window(priv->invisible);
    timestamp = gdk_x11_get_server_time(window);

    priv->screen = screen;

    kiran_x11_manager_set_orientation_property(manager);
    kiran_x11_manager_set_visual_property(manager);
    kiran_x11_manager_set_padding_property(manager);
    kiran_x11_manager_set_icon_size_property(manager);
    kiran_x11_manager_set_colors_property(manager);

    /* 检查我们是否能够成功设置选择区 */
    if (gdk_selection_owner_set_for_display(display,
                                            window,
                                            priv->selection_atom,
                                            timestamp,
                                            TRUE))
    {
        XClientMessageEvent xev;

        xev.type = ClientMessage;
        xev.window = RootWindowOfScreen(xscreen);
        xev.message_type = gdk_x11_get_xatom_by_name_for_display(display,
                                                                 "MANAGER");

        xev.format = 32;
        xev.data.l[0] = timestamp;
        xev.data.l[1] = gdk_x11_atom_to_xatom_for_display(display,
                                                          priv->selection_atom);
        xev.data.l[2] = GDK_WINDOW_XID(window);
        xev.data.l[3] = 0;
        xev.data.l[4] = 0;

        XSendEvent(GDK_DISPLAY_XDISPLAY(display),
                   RootWindowOfScreen(xscreen),
                   False, StructureNotifyMask, (XEvent *)&xev);

        gdk_window_add_filter(window,
                              (GdkFilterFunc)kiran_x11_tray_manager_window_filter, manager);
        return TRUE;
    }
    else
    {
        gtk_widget_destroy(priv->invisible);
        g_object_unref(priv->invisible);
        priv->screen = NULL;
    }

    return FALSE;
}

KiranTrayManager *
kiran_x11_tray_manager_new(GdkScreen *screen)
{
    KiranX11TrayManager *manager = g_object_new(KIRAN_TYPE_X11_TRAY_MANAGER, NULL);

    kiran_x11_tray_manager_manage_screen(manager, screen);

    return KIRAN_TRAY_MANAGER(manager);
}
