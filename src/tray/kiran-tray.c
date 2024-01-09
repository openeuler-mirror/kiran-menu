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

#include "kiran-tray.h"
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "kiran-notify-icon-window.h"
#include "kiran-notify-icon.h"
#include "kiran-sn-manager-gen.h"
#include "kiran-sn-tray-manager.h"
#include "kiran-x11-tray-manager.h"
#include "kiran-x11-tray-socket.h"
#include "tray-i.h"
#include "kiran-x11-tray-icon.h"

#define ROOT_NODE_NAME "apps"
#define APP_NODE_NAME "app"
#define APP_ID_NODE_NAME "id"
#define APP_ICON_NODE_NAME "icon"         /*应用图标 */
#define APP_TITLE_NODE_NAME "title"       /*应用名称*/
#define APP_CATEGORY_NODE_NAME "category" /*应用分类 */

struct _KiranTrayPrivate
{
    GtkWidget *icons_win_button;
    GtkWidget *icons_win;
    GSList *icons;
    GSList *managers;
    GSettings *settings;
    KiranTrayLoaction location;

    guint bus_name_id;
    KiranSnManagerGenSkeleton *skeleton;
};

static void kiran_tray_finalize(GObject *object);
static GObject *kiran_tray_constructor(GType type,
                                       guint n_construct_properties,
                                       GObjectConstructParam *construct_properties);
static void kiran_tray_realize(GtkWidget *widget);
static void kiran_tray_unrealize(GtkWidget *widget);

static void kiran_tray_add_manager(KiranTray *tray, KiranTrayManager *manager);
static void kiran_tray_icons_refresh(KiranTray *tray);
static void position_notify_icon_window(KiranTray *tray, gboolean change_y);
static void kiran_tray_style_updated(GtkWidget *widget);
static gchar *get_widget_geometry(GtkWidget *widget);

G_DEFINE_TYPE_WITH_PRIVATE(KiranTray, kiran_tray, GTK_TYPE_BOX)

static void
kiran_tray_set_icon_size(KiranTray *tray)
{
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;
    gint icon_size = 0;

    priv = tray->priv;
    icon_size = g_settings_get_int(priv->settings, KEY_TRAY_ICON_SIZE);

    for (iterator = priv->managers; iterator; iterator = iterator->next)
    {
        kiran_tray_manager_set_icon_size(iterator->data, icon_size);
    }

    kiran_sn_manager_gen_set_icon_size(KIRAN_SN_MANAGER_GEN(priv->skeleton), icon_size);
}

static void
kiran_tray_set_icon_padding(KiranTray *tray)
{
    KiranTrayPrivate *priv;
    gint icon_padding = 2;

    priv = tray->priv;
    icon_padding = g_settings_get_int(priv->settings, KEY_TRAY_ICON_PADDING);

    gtk_box_set_spacing(GTK_BOX(tray), icon_padding);
}

static KiranNotifyIcon *
kiran_tray_find_icon_by_id(KiranTray *tray,
                           const gchar *id)
{
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;

    priv = tray->priv;

    for (iterator = priv->icons; iterator; iterator = iterator->next)
    {
        const gchar *icon_id;

        if (!KIRAN_IS_NOTIFY_ICON(iterator->data))
            continue;

        icon_id = kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(iterator->data));
        if (g_strcmp0(icon_id, id) == 0)
        {
            return iterator->data;
        }
    }

    return NULL;
}

static void
gsettings_changed_panel_icon_ids(GSettings *settings,
                                 gchar *key,
                                 KiranTray *tray)
{
    KiranTrayPrivate *priv = tray->priv;
    gchar **panel_icon_ids;
    gboolean find = TRUE;
    gint i;
    GList *children;
    GList *child;

    panel_icon_ids = g_settings_get_strv(settings, KEY_PANEL_ICON_IDS);
    children = gtk_container_get_children(GTK_CONTAINER(tray));

    if (panel_icon_ids)
    {
        for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
        {
            KiranNotifyIcon *icon;
            icon = kiran_tray_find_icon_by_id(tray, panel_icon_ids[i]);

            if (icon)
            {
                find = FALSE;

                for (child = children; child; child = child->next)
                {
                    const gchar *id;

                    if (!KIRAN_IS_NOTIFY_ICON(child->data))
                        continue;

                    id = kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(child->data));

                    if (g_strcmp0(id, panel_icon_ids[i]) == 0)
                    {
                        find = TRUE;
                        break;
                    }
                }

                //remove from tray to panel
                if (!find)
                {
                    KiranNotifyIconWay way = kiran_notify_icon_get_way(icon);

                    if (way == KIRAN_NOTIFY_ICON_WAY_X11)
                        kiran_notify_icon_window_remove_icon(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win), icon);
                    else if (way == KIRAN_NOTIFY_ICON_WAY_DBUS)
                    {
                        g_object_ref(icon);
                        kiran_notify_icon_window_remove_icon(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win), icon);
                        gtk_box_pack_start(GTK_BOX(tray), GTK_WIDGET(icon), FALSE, TRUE, 0);
                        kiran_tray_icons_refresh(tray);
                    }

                    if (kiran_notify_icon_window_get_icons_number(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win)) <= 0)
                    {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->icons_win_button), FALSE);
                        gtk_widget_hide(priv->icons_win);
                        gtk_widget_hide(priv->icons_win_button);
                    }
                }
            }
        }

        for (child = children; child; child = child->next)
        {
            const gchar *id;

            if (!KIRAN_IS_NOTIFY_ICON(child->data))
                continue;

            id = kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(child->data));
            find = FALSE;
            for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
            {
                if (g_strcmp0(id, panel_icon_ids[i]) == 0)
                {
                    find = TRUE;
                    break;
                }
            }

            //remove from panel to tray
            if (!find)
            {
                KiranNotifyIconWay way = kiran_notify_icon_get_way(child->data);

                if (way == KIRAN_NOTIFY_ICON_WAY_X11)
                    gtk_container_remove(GTK_CONTAINER(tray), GTK_WIDGET(child->data));
                else if (way == KIRAN_NOTIFY_ICON_WAY_DBUS)
                {
                    g_object_ref(child->data);
                    gtk_container_remove(GTK_CONTAINER(tray), GTK_WIDGET(child->data));
                    kiran_notify_icon_window_add_icon(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win), child->data);
                    gtk_widget_show(priv->icons_win_button);
                }
            }
        }

        position_notify_icon_window(tray, FALSE);

        g_strfreev(panel_icon_ids);
    }
    g_list_free(children);
}

static void
gsettings_changed_panel_icon_size(GSettings *settings,
                                  gchar *key,
                                  KiranTray *tray)
{
    kiran_tray_set_icon_size(tray);
}

static void
gsettings_changed_panel_icon_padding(GSettings *settings,
                                     gchar *key,
                                     KiranTray *tray)
{
    kiran_tray_set_icon_padding(tray);
}

static void
kiran_tray_class_init(KiranTrayClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->finalize = kiran_tray_finalize;
    gobject_class->constructor = kiran_tray_constructor;
    widget_class->realize = kiran_tray_realize;
    widget_class->unrealize = kiran_tray_unrealize;
    widget_class->style_updated = kiran_tray_style_updated;
}

static void
bus_acquired_cb(GDBusConnection *connection,
                const gchar *name,
                gpointer user_data)
{
    KiranTray *tray;
    GDBusInterfaceSkeleton *skeleton;
    GError *error;

    tray = KIRAN_TRAY(user_data);
    skeleton = G_DBUS_INTERFACE_SKELETON(tray->priv->skeleton);

    error = NULL;
    g_dbus_interface_skeleton_export(skeleton, connection,
                                     "/StatusNotifierManager", &error);

    if (error != NULL)
    {
        g_warning("%s", error->message);
        g_error_free(error);
    }
}

static gboolean
handle_get_geometry(KiranSnManagerGenSkeleton *skeleton,
                    GDBusMethodInvocation *invocation,
                    const gchar *arg_id,
                    gpointer user_data)
{
    KiranTray *tray;
    KiranTrayPrivate *priv;
    KiranNotifyIcon *icon;
    gchar *geometry;

    tray = KIRAN_TRAY(user_data);
    priv = tray->priv;

    icon = kiran_tray_find_icon_by_id(tray,
                                      arg_id);

    if (icon == NULL)
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR,
                                              G_DBUS_ERROR_INVALID_ARGS,
                                              "icon id '%s' is not valid",
                                              arg_id);

        return TRUE;
    }

    geometry = get_widget_geometry(GTK_WIDGET(icon));
    kiran_sn_manager_gen_complete_get_geometry(KIRAN_SN_MANAGER_GEN(skeleton),
                                               invocation,
                                               geometry);
    g_free(geometry);

    return TRUE;
}

static JsonObject *
get_color_json_object(GdkRGBA *color)
{
    JsonObject *object;

    object = json_object_new();

    json_object_set_int_member(object,
                               "red",
                               color->red);

    json_object_set_int_member(object,
                               "green",
                               color->green);

    json_object_set_int_member(object,
                               "blue",
                               color->blue);

    json_object_set_int_member(object,
                               "alpha",
                               color->alpha);

    return object;
}

static gchar *
get_widget_style(GtkWidget *widget)
{
    JsonGenerator *generator;
    JsonNode *root;
    JsonObject *object;
    GtkStyleContext *context;
    GdkRGBA fg;
    GdkRGBA error;
    GdkRGBA warning;
    GdkRGBA success;
    gchar *data;

    context = gtk_widget_get_style_context(widget);
    gtk_style_context_save(context);
    gtk_style_context_set_state(context, GTK_STATE_FLAG_NORMAL);

    gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, &fg);

    if (!gtk_style_context_lookup_color(context, "error_color", &error))
        error = fg;
    if (!gtk_style_context_lookup_color(context, "warning_color", &warning))
        warning = fg;
    if (!gtk_style_context_lookup_color(context, "success_color", &success))
        success = fg;

    gtk_style_context_restore(context);

    generator = json_generator_new();
    root = json_node_new(JSON_NODE_OBJECT);
    object = json_object_new();

    json_node_init_object(root, object);
    json_generator_set_root(generator, root);

    json_object_set_object_member(object,
                                  "fg_color",
                                  get_color_json_object(&fg));

    json_object_set_object_member(object,
                                  "error_color",
                                  get_color_json_object(&error));

    json_object_set_object_member(object,
                                  "warning_color",
                                  get_color_json_object(&warning));

    json_object_set_object_member(object,
                                  "success_color",
                                  get_color_json_object(&success));

    json_node_init_object(root, object);
    json_generator_set_root(generator, root);

    data = json_generator_to_data(generator, NULL);

    json_object_unref(object);
    json_node_free(root);
    g_object_unref(generator);

    return data;
}

static gboolean
handle_get_style(KiranSnManagerGenSkeleton *skeleton,
                 GDBusMethodInvocation *invocation,
                 gpointer user_data)
{
    KiranTray *tray;
    gchar *style;

    tray = KIRAN_TRAY(user_data);

    style = get_widget_style(GTK_WIDGET(tray));

    kiran_sn_manager_gen_complete_get_style(KIRAN_SN_MANAGER_GEN(skeleton),
                                            invocation,
                                            style);

    g_free(style);

    return TRUE;
}

static void
kiran_tray_init(KiranTray *self)
{
    GdkDisplay *display;
    GtkCssProvider *provider;
    GdkScreen *screen;
    GBusNameOwnerFlags flags;
    KiranTrayPrivate *priv;

    priv = self->priv = kiran_tray_get_instance_private(self);
    priv->settings = g_settings_new(KIRAN_TRAY_SCHEMA);
    g_signal_connect(priv->settings, "changed::" KEY_PANEL_ICON_IDS, G_CALLBACK(gsettings_changed_panel_icon_ids), self);
    g_signal_connect(priv->settings, "changed::" KEY_TRAY_ICON_SIZE, G_CALLBACK(gsettings_changed_panel_icon_size), self);
    g_signal_connect(priv->settings, "changed::" KEY_TRAY_ICON_PADDING, G_CALLBACK(gsettings_changed_panel_icon_padding), self);
    priv->location = APPLET_ORIENT_UP;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_resource(provider, "/kiran-applet/css/applet.css");

    gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_START);
    gtk_widget_set_hexpand(GTK_WIDGET(self), FALSE);
    g_object_unref(provider);

    priv->skeleton = g_object_new(KIRAN_SN_TYPE_MANAGER_GEN_SKELETON, NULL);

    flags = G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
            G_BUS_NAME_OWNER_FLAGS_REPLACE;
    priv->bus_name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                       "org.kde.StatusNotifierManager",
                                       flags,
                                       bus_acquired_cb,
                                       NULL, NULL, self, NULL);
    g_signal_connect(priv->skeleton,
                     "handle-get-geometry",
                     G_CALLBACK(handle_get_geometry),
                     self);

    g_signal_connect(priv->skeleton,
                     "handle-get-style",
                     G_CALLBACK(handle_get_style),
                     self);
}

static void
kiran_tray_icons_refresh(KiranTray *tray)
{
    GList *children;
    GList *child;
    gint col = 0;

    children = gtk_container_get_children(GTK_CONTAINER(tray));
    children = g_list_sort(children, (GCompareFunc)kiran_notify_icon_compare);

    for (child = children; child; child = child->next)
    {
        gtk_container_child_set(GTK_CONTAINER(tray),
                                child->data,
                                "position", col,
                                NULL);
        col++;
    }
    g_list_free(children);
}

static void
position_notify_icon_window(KiranTray *tray, gboolean change_y)
{
    GtkAllocation allocation;
    GdkDisplay *display;
    GdkScreen *screen;
    GdkRectangle monitor;
    GdkGravity gravity = GDK_GRAVITY_NORTH_WEST;
    int button_w, button_h;
    int x, y;
    int w, h;
    int i, n;
    gboolean found_monitor = FALSE;
    KiranTrayPrivate *priv = tray->priv;

    if (!GDK_IS_X11_DISPLAY(gdk_display_get_default()))
        return;

    if (!gtk_widget_is_visible(priv->icons_win))
        return;

    /* Get root origin of the toggle button, and position above that. */
    gdk_window_get_origin(gtk_widget_get_window(priv->icons_win_button),
                          &x, &y);

    gtk_widget_get_size_request(GTK_WIDGET(priv->icons_win), &w, &h);

    gtk_widget_get_allocation(priv->icons_win_button, &allocation);
    button_w = allocation.width;
    button_h = allocation.height;

    screen = gtk_window_get_screen(GTK_WINDOW(priv->icons_win));
    display = gdk_screen_get_display(screen);

    n = gdk_display_get_n_monitors(display);
    for (i = 0; i < n; i++)
    {
        gdk_monitor_get_geometry(gdk_display_get_monitor(display, i), &monitor);
        if (x >= monitor.x && x <= monitor.x + monitor.width &&
            y >= monitor.y && y <= monitor.y + monitor.height)
        {
            found_monitor = TRUE;
            break;
        }
    }

    if (!found_monitor)
    {
        /* eek, we should be on one of those xinerama
               monitors */
        monitor.x = 0;
        monitor.y = 0;
        monitor.width = WidthOfScreen(gdk_x11_screen_get_xscreen(screen));
        monitor.height = HeightOfScreen(gdk_x11_screen_get_xscreen(screen));
    }

    /* Based on panel orientation, position the popup.
     * Ignore window gravity since the window is undecorated.
     * The orientations are all named backward from what
     * I expected.
     */
    switch (priv->location)
    {
    case APPLET_ORIENT_RIGHT:
        x += button_w;
        if ((y + h) > monitor.y + monitor.height)
            y -= (y + h) - (monitor.y + monitor.height);

        if ((y + h) > (monitor.height / 2))
            gravity = GDK_GRAVITY_SOUTH_WEST;
        else
            gravity = GDK_GRAVITY_NORTH_WEST;

        break;
    case APPLET_ORIENT_LEFT:
        x -= w;
        if ((y + h) > monitor.y + monitor.height)
            y -= (y + h) - (monitor.y + monitor.height);

        if ((y + h) > (monitor.height / 2))
            gravity = GDK_GRAVITY_SOUTH_EAST;
        else
            gravity = GDK_GRAVITY_NORTH_EAST;

        break;
    case APPLET_ORIENT_DOWN:
        y += button_h;
        if ((x + w) > monitor.x + monitor.width)
            x -= (x + w) - (monitor.x + monitor.width);

        x -= (w - button_w) / 2;
        gravity = GDK_GRAVITY_NORTH_WEST;

        break;
    case APPLET_ORIENT_UP:
        y -= h;
        if ((x + w) > monitor.x + monitor.width)
            x -= (x + w) - (monitor.x + monitor.width);

        x -= (w - button_w) / 2;

        gravity = GDK_GRAVITY_SOUTH_WEST;

        break;
    }

    //避免移出屏幕外
    if (x < 0)
        x = 0;

    if (y < 0)
        y = 0;

    if (change_y)
        gtk_window_move(GTK_WINDOW(priv->icons_win), x, y);
    else
    {
        gint root_x;
        gint root_y;

        gtk_window_get_position(GTK_WINDOW(priv->icons_win), &root_x, &root_y);
        gtk_window_move(GTK_WINDOW(priv->icons_win), x, root_y);
    }

    gtk_window_set_gravity(GTK_WINDOW(priv->icons_win), gravity);
}

static void
icons_win_button_toggled_cb(GtkWidget *widget,
                            gpointer user_data)
{
    KiranTray *tray = KIRAN_TRAY(user_data);
    KiranTrayPrivate *priv = tray->priv;

    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tray->priv->icons_win_button)))
    {
        gtk_widget_hide(priv->icons_win);
    }
    else
    {
        gtk_widget_show(priv->icons_win);
        position_notify_icon_window(tray, TRUE);
    }
}

static void
icons_win_hide_cb(GtkWidget *widget,
                  gpointer user_data)
{
    KiranTray *tray = KIRAN_TRAY(user_data);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tray->priv->icons_win_button), FALSE);
}

static void
kiran_tray_finalize(GObject *object)
{
    KiranTray *tray = KIRAN_TRAY(object);
    KiranTrayPrivate *priv = tray->priv;

    g_signal_handlers_disconnect_by_func(priv->settings,
                                         G_CALLBACK(gsettings_changed_panel_icon_ids),
                                         tray);

    g_signal_handlers_disconnect_by_func(priv->settings,
                                         G_CALLBACK(gsettings_changed_panel_icon_size),
                                         tray);

    g_signal_handlers_disconnect_by_func(priv->settings,
                                         G_CALLBACK(gsettings_changed_panel_icon_padding),
                                         tray);
    if (priv->bus_name_id > 0)
    {
        g_bus_unown_name(priv->bus_name_id);
        priv->bus_name_id = 0;
    }

    g_object_unref(priv->settings);
    priv->settings = NULL;

    gtk_widget_destroy(priv->icons_win);
    priv->icons_win = NULL;

    G_OBJECT_CLASS(kiran_tray_parent_class)->finalize(object);
}

static GObject *
kiran_tray_constructor(GType type,
                       guint n_construct_properties,
                       GObjectConstructParam *construct_properties)
{
    GObject *obj;
    KiranTray *tray;

    obj = G_OBJECT_CLASS(kiran_tray_parent_class)->constructor(type, n_construct_properties, construct_properties);
    tray = KIRAN_TRAY(obj);
    gtk_box_set_homogeneous(GTK_BOX(tray), FALSE);
    gtk_widget_set_name(GTK_WIDGET(tray), "trayBox");
    kiran_tray_set_icon_padding(tray);

    tray->priv->icons_win = kiran_notify_icon_window_new();
    tray->priv->icons_win_button = gtk_toggle_button_new();
    GtkWidget *arrow_image = gtk_image_new_from_icon_name("kiran-menu-arrow-up-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(tray->priv->icons_win_button), arrow_image);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tray->priv->icons_win_button), TRUE);
    gtk_widget_set_name(tray->priv->icons_win_button, "iconWinButton");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tray->priv->icons_win_button), FALSE);

    g_signal_connect(tray->priv->icons_win_button, "toggled", G_CALLBACK(icons_win_button_toggled_cb), tray);
    g_signal_connect(tray->priv->icons_win, "hide", G_CALLBACK(icons_win_hide_cb), tray);

    gtk_box_pack_start(GTK_BOX(tray), GTK_WIDGET(tray->priv->icons_win_button), FALSE, TRUE, 0);

    return obj;
}

static void
kiran_tray_realize(GtkWidget *widget)
{
    KiranTray *tray = KIRAN_TRAY(widget);
    KiranTrayManager *manager;

    GTK_WIDGET_CLASS(kiran_tray_parent_class)->realize(widget);

    manager = kiran_x11_tray_manager_new(gtk_widget_get_screen(widget));

    kiran_tray_add_manager(tray, manager);

    manager = kiran_sn_tray_manager_new();
    kiran_tray_add_manager(tray, manager);

    kiran_tray_set_icon_size(tray);
}

static void
kiran_tray_unrealize(GtkWidget *widget)
{
    KiranTrayPrivate *priv = KIRAN_TRAY(widget)->priv;

    if (priv->managers)
    {
        g_slist_free_full(priv->managers, g_object_unref);
        priv->managers = NULL;
    }

    g_clear_pointer(&priv->icons, g_slist_free);

    GTK_WIDGET_CLASS(kiran_tray_parent_class)->unrealize(widget);
}

static void
kiran_tray_style_updated(GtkWidget *widget)
{
    KiranTray *tray;
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;
    GtkStyleContext *context;
    gchar *style;

    if (GTK_WIDGET_CLASS(kiran_tray_parent_class)->style_updated)
        GTK_WIDGET_CLASS(kiran_tray_parent_class)->style_updated(widget);

    tray = KIRAN_TRAY(widget);
    priv = tray->priv;
    context = gtk_widget_get_style_context(widget);

    for (iterator = priv->managers; iterator; iterator = iterator->next)
    {
        kiran_tray_manager_style_updated(iterator->data, context);
    }

    style = get_widget_style(GTK_WIDGET(tray));
    kiran_sn_manager_gen_emit_style_changed(KIRAN_SN_MANAGER_GEN(priv->skeleton),
                                            style);
    g_free(style);
}

static gint
kiran_tray_get_icon_type(GSettings *settings, const char *id)
{
    gchar **panel_icon_ids;
    gchar **hide_icon_ids;
    gint ret = ICON_SHOW_IN_WINDOW;
    gint i;

    if (!id)
        return ret;

    hide_icon_ids = g_settings_get_strv(settings, KEY_HIDE_ICON_IDS);
    if (hide_icon_ids)
    {
        for (i = 0; hide_icon_ids && hide_icon_ids[i]; i++)
        {
            if (g_strcmp0(id, hide_icon_ids[i]) == 0)
            {
                g_strfreev(hide_icon_ids);
                return ICON_NOT_SHOW;
            }
        }
        g_strfreev(hide_icon_ids);
    }

    panel_icon_ids = g_settings_get_strv(settings, KEY_PANEL_ICON_IDS);
    if (panel_icon_ids)
    {
        for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
        {
            if (g_strcmp0(id, panel_icon_ids[i]) == 0)
            {
                ret = ICON_SHOW_IN_PANEL;
            }
        }
        g_strfreev(panel_icon_ids);
    }

    return ret;
}

static xmlNodePtr
create_app_node(KiranNotifyIcon *icon)
{
    xmlNodePtr node;
    const char *id;
    const char *icon_name;
    const char *name;
    const char *app_category;

    id = kiran_notify_icon_get_id(icon);
    icon_name = kiran_notify_icon_get_icon(icon);
    name = kiran_notify_icon_get_name(icon);
    app_category = kiran_notify_icon_get_app_category(icon);

    node = xmlNewNode(NULL, (const xmlChar *)APP_NODE_NAME);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_ID_NODE_NAME, (const xmlChar *)id);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_ICON_NODE_NAME, (const xmlChar *)icon_name);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_TITLE_NODE_NAME, (const xmlChar *)name);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_CATEGORY_NODE_NAME, (const xmlChar *)app_category);

    return node;
}

static void
record_tray_notify_icon(KiranNotifyIcon *icon)
{
    gchar *docname = NULL;
    xmlDocPtr doc;
    xmlNodePtr root_node;
    xmlNodePtr node;
    const char *id;

    id = kiran_notify_icon_get_id(icon);

    docname = g_strdup_printf("%s/.config/kiran-tray/apps.xml", g_get_home_dir());
    if (docname == NULL)
        return;

    if (!g_file_test(docname, G_FILE_TEST_EXISTS))
    {
        //文件不存在
        gchar *dir = NULL;

        dir = g_strdup_printf("%s/.config", g_get_home_dir());
        if (dir == NULL)
        {
            g_free(docname);
            return;
        }

        if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        {
            g_mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        free(dir);

        dir = g_strdup_printf("%s/.config/kiran-tray", g_get_home_dir());
        if (dir == NULL)
        {
            g_free(docname);
            return;
        }

        if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        {
            g_mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        free(dir);
    }

    doc = NULL;
    if (g_file_test(docname, G_FILE_TEST_EXISTS))
        doc = xmlReadFile(docname, NULL, XML_PARSE_NOBLANKS);

    if (doc == NULL)
    {
        doc = xmlNewDoc((const xmlChar *)"1.0");
        root_node = xmlNewNode(NULL, (const xmlChar *)ROOT_NODE_NAME);
        node = create_app_node(icon);
        xmlAddChild(root_node, node);

        xmlDocSetRootElement(doc, root_node);
        xmlSaveFormatFileEnc(docname, doc, "UTF-8", 1);
    }
    else
    {
        root_node = xmlDocGetRootElement(doc);
        if (root_node == NULL)
        {
            if (g_remove(docname) == 0)
            {
                record_tray_notify_icon(icon);
            }
        }
        else
        {
            if (xmlStrcmp(root_node->name, (const xmlChar *)ROOT_NODE_NAME))
            {
                record_tray_notify_icon(icon);
            }
            else
            {
                gboolean is_exist = FALSE;

                node = root_node->xmlChildrenNode;
                while (node != NULL)
                {
                    if (!xmlStrcmp(node->name, (const xmlChar *)APP_NODE_NAME))
                    {
                        xmlNodePtr id_node;

                        id_node = node->xmlChildrenNode;
                        while (id_node != NULL)
                        {
                            if (!xmlStrcmp(id_node->name, (const xmlChar *)APP_ID_NODE_NAME))
                            {
                                if (!xmlStrcmp(xmlNodeGetContent(id_node), (const xmlChar *)id))
                                {
                                    is_exist = TRUE;
                                    break;
                                }
                            }
                            id_node = id_node->next;
                        }
                    }

                    node = node->next;
                }

                if (!is_exist)
                {
                    node = create_app_node(icon);
                    xmlAddChild(root_node, node);
                    xmlSaveFormatFileEnc(docname, doc, "UTF-8", 1);
                }
            }
        }
    }

    //关闭文档指针，并清除文档中所有节点动态申请的内存
    xmlFreeDoc(doc);
    g_free(docname);
}

static void
kiran_tray_notify_icon_map_callback(GtkWidget *widget,
                                    gpointer user_data)
{
    KiranNotifyIcon *icon = KIRAN_NOTIFY_ICON(widget);

    /* 当窗口保存图标时才进行记录 */
    if (kiran_notify_icon_get_icon(icon))
    {
        record_tray_notify_icon(icon);
    }
}

static gchar *
get_widget_geometry(GtkWidget *widget)
{
    GdkWindow *window;
    GtkWidget *parent;
    GtkAllocation allocation;
    JsonGenerator *generator;
    JsonNode *root;
    JsonObject *object;
    gchar *data;
    int x, y, width, height;

    x = y = width = height = 0;

    parent = gtk_widget_get_parent(widget);
    /* 获取顶层窗口的位置 */
    while (parent)
    {
        window = gtk_widget_get_window(parent);
        {
            gdk_window_get_origin(window, &x, &y);
        }
        parent = gtk_widget_get_parent(parent);
    }

    gtk_widget_get_allocation(widget, &allocation);
    x += allocation.x;
    y += allocation.y;
    width = allocation.width;
    height = allocation.height;

    generator = json_generator_new();
    root = json_node_new(JSON_NODE_OBJECT);
    object = json_object_new();

    json_object_set_int_member(object,
                               "x",
                               x);

    json_object_set_int_member(object,
                               "y",
                               y);

    json_object_set_int_member(object,
                               "width",
                               width);

    json_object_set_int_member(object,
                               "height",
                               height);

    json_node_init_object(root, object);
    json_generator_set_root(generator, root);

    data = json_generator_to_data(generator, NULL);

    json_object_unref(object);
    json_node_free(root);
    g_object_unref(generator);

    return data;
}

void kiran_tray_resize_x11_icon_window(GdkDisplay *display,Window icon_window,GtkWidget *widget)
{
    XWindowAttributes window_attributes;
    Status status = XGetWindowAttributes(GDK_DISPLAY_XDISPLAY(display),icon_window,&window_attributes);
    if(status == 0)
    {
        g_info("get window attributes failed");
        return;
    }
    g_debug("window attributes: widget: %d, height:%d ",window_attributes.width , window_attributes.height);
    
    GtkAllocation icon_allocation;
    gtk_widget_get_allocation(widget, &icon_allocation);
    g_debug("icon container allocation height:%d",icon_allocation.height);

    /**
     * 这里只将window的高与图标容器的高保持一致，暂不限制window的宽 (#22117)
    */
    if(window_attributes.height != icon_allocation.height)
    {
        g_debug("resize X window");
        XResizeWindow(GDK_DISPLAY_XDISPLAY(display),icon_window,window_attributes.width,icon_allocation.height);
        XFlush(GDK_DISPLAY_XDISPLAY(display));
    }
}

static void
icon_size_allocate_callback(GtkWidget *widget,
                            GdkRectangle *allocation,
                            gpointer user_data)
{
    KiranTray *tray;
    KiranTrayPrivate *priv;
    gchar *geometry;
    const char *id;

    if (!gtk_widget_is_visible(widget))
        return;

    tray = KIRAN_TRAY(user_data);
    priv = tray->priv;

    geometry = get_widget_geometry(widget);
    
    id = kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(widget));
    g_debug("icon size allocate changed: id:%s, %s",id ? id : "NULL",geometry);

    kiran_sn_manager_gen_emit_geometry_changed(KIRAN_SN_MANAGER_GEN(priv->skeleton),
                                               kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(widget)),
                                               geometry);
    g_free(geometry);

    if(KIRAN_IS_X11_TRAY_ICON(KIRAN_X11_TRAY_ICON(widget)))
    {
        Window icon_window;
        icon_window = kiran_x11_tray_icon_get_icon_window(KIRAN_X11_TRAY_ICON(widget));
        g_debug("%s is x11 tray icon, window id : %d",id ? id : "NULL",icon_window);

        if(!icon_window)
        {
            return;
        }

        GdkDisplay *display;
        GdkScreen *screen;
        screen = gtk_widget_get_screen(GTK_WIDGET(user_data));
        display = gdk_screen_get_display(screen);

        gdk_x11_display_error_trap_push(display);

        kiran_tray_resize_x11_icon_window(display,icon_window,widget);

        gdk_x11_display_error_trap_pop(display);
    }   
}

static gboolean
icon_button_press_event_callback(GtkWidget *widget,
                                 GdkEventButton *event,
                                 gpointer user_data)
{
    KiranTray *tray;
    KiranTrayPrivate *priv;
    GdkWindow *window;
    GdkDisplay *display;
    GdkSeat *seat;
    GdkDevice *pointer;
    enum PointerEventType type;
    gint pointer_x, pointer_y;

    tray = KIRAN_TRAY(user_data);
    priv = tray->priv;

    window = gtk_widget_get_window(widget);
    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);
    pointer = gdk_seat_get_pointer(seat);
    gdk_device_get_position(pointer, NULL, &pointer_x, &pointer_y);

    switch (event->button)
    {
    case 1:
        type = POINTER_EVENT_TYPE_LEFT_BUTTON_PRESS;
        break;

    case 2:
        type = POINTER_EVENT_TYPE_MIDDLE_BUTTON_PRESS;
        break;

    default:
        type = POINTER_EVENT_TYPE_RIGHT_BUTTON_PRESS;
        break;
    }

    kiran_sn_manager_gen_emit_point_clicked(KIRAN_SN_MANAGER_GEN(priv->skeleton),
                                            kiran_notify_icon_get_id(KIRAN_NOTIFY_ICON(widget)),
                                            type,
                                            pointer_x,
                                            pointer_y);

    return FALSE;
}

static void
kiran_tray_notify_icon_added(KiranTrayManager *manager,
                             KiranNotifyIcon *icon,
                             KiranTray *tray)
{
    KiranTrayPrivate *priv;
    const char *id;
    gint type;

    g_return_if_fail(KIRAN_IS_TRAY_MANAGER(manager));
    g_return_if_fail(KIRAN_IS_NOTIFY_ICON(icon));
    g_return_if_fail(KIRAN_IS_TRAY(tray));

    priv = tray->priv;

    id = kiran_notify_icon_get_id(icon);
    type = kiran_tray_get_icon_type(priv->settings, id);

    /* 系统已知的通知图标如声音，网络，电池等不需要记录 */
    if (kiran_notify_icon_get_category(icon) != KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE)
    {
        g_signal_connect(icon,
                         "map",
                         G_CALLBACK(kiran_tray_notify_icon_map_callback),
                         NULL);
    }

    gtk_widget_set_name(GTK_WIDGET(icon), "iconWinButton");

    if (type != ICON_NOT_SHOW)
    {
        gtk_box_pack_start(GTK_BOX(tray), GTK_WIDGET(icon), FALSE, TRUE, 0);
        kiran_tray_icons_refresh(tray);
    }

    /*
    if (type == ICON_SHOW_IN_PANEL)
    {
        gtk_box_pack_start(GTK_BOX(tray), GTK_WIDGET(icon), FALSE, TRUE, 0);
        kiran_tray_icons_refresh(tray);
    }
    else if (type == ICON_SHOW_IN_WINDOW)
    {
        kiran_notify_icon_window_add_icon(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win), icon);

        if (!gtk_widget_is_visible(priv->icons_win_button))
            gtk_widget_show(priv->icons_win_button);
        position_notify_icon_window(tray, FALSE);
    }
    */

    g_signal_connect(icon,
                     "size-allocate",
                     G_CALLBACK(icon_size_allocate_callback),
                     tray);

    g_signal_connect(icon,
                     "button-press-event",
                     G_CALLBACK(icon_button_press_event_callback),
                     tray);

    priv->icons = g_slist_prepend(priv->icons, icon);
}

static void
kiran_tray_notify_icon_removed(KiranTrayManager *manager,
                               KiranNotifyIcon *icon,
                               KiranTray *tray)
{
    KiranTrayPrivate *priv;
    const char *id;
    gint type;

    g_return_if_fail(KIRAN_IS_TRAY_MANAGER(manager));
    g_return_if_fail(KIRAN_IS_NOTIFY_ICON(icon));
    g_return_if_fail(KIRAN_IS_TRAY(tray));

    priv = tray->priv;

    id = kiran_notify_icon_get_id(icon);
    type = kiran_tray_get_icon_type(priv->settings, id);

    if (type != ICON_NOT_SHOW)
    {
        gtk_container_remove(GTK_CONTAINER(tray), GTK_WIDGET(icon));
    }

    /*
    if (type == ICON_SHOW_IN_PANEL)
    {
        gtk_container_remove(GTK_CONTAINER(tray), GTK_WIDGET(icon));
    }
    else if (type == ICON_SHOW_IN_WINDOW)
    {
        kiran_notify_icon_window_remove_icon(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win), icon);

        if (kiran_notify_icon_window_get_icons_number(KIRAN_NOTIFY_ICON_WINDOW(priv->icons_win)) <= 0)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->icons_win_button), FALSE);
            gtk_widget_hide(priv->icons_win);
            gtk_widget_hide(priv->icons_win_button);
        }

        position_notify_icon_window(tray, FALSE);
    }
    */

    priv->icons = g_slist_remove(priv->icons, icon);
}

static void
kiran_tray_add_manager(KiranTray *tray, KiranTrayManager *manager)
{
    KiranTrayPrivate *priv = tray->priv;

    priv->managers = g_slist_prepend(priv->managers, manager);

    g_signal_connect_object(manager, "icon-added",
                            G_CALLBACK(kiran_tray_notify_icon_added), tray, 0);
    g_signal_connect_object(manager, "icon-removed",
                            G_CALLBACK(kiran_tray_notify_icon_removed), tray, 0);
}

GtkWidget *
kiran_tray_new(void)
{
    return g_object_new(KIRAN_TYPE_TRAY, NULL);
}
