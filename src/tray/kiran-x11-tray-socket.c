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

#include "kiran-x11-tray-socket.h"

struct _KiranX11TraySocketPrivate
{
    gboolean has_alpha;
};

G_DEFINE_TYPE_WITH_PRIVATE(KiranX11TraySocket, kiran_x11_tray_socket, GTK_TYPE_SOCKET)

static void
kiran_x11_tray_socket_realize(GtkWidget *widget)
{
    KiranX11TraySocket *socket = KIRAN_X11_TRAY_SOCKET(widget);
    KiranX11TraySocketPrivate *priv = socket->priv;
    GdkWindow *window;

    GTK_WIDGET_CLASS(kiran_x11_tray_socket_parent_class)->realize(widget);

    window = gtk_widget_get_window(widget);

    if (priv->has_alpha)
    {
        cairo_pattern_t *transparent = cairo_pattern_create_rgba(0, 0, 0, 0);
        gdk_window_set_background_pattern(window, transparent);
        gdk_window_set_composited(window, TRUE);
        cairo_pattern_destroy(transparent);
    }

    gtk_widget_set_app_paintable(widget, priv->has_alpha);
}

static gboolean
kiran_x11_tray_socket_draw(GtkWidget *widget,
                           cairo_t *cr)
{
    KiranX11TraySocket *socket = KIRAN_X11_TRAY_SOCKET(widget);
    KiranX11TraySocketPrivate *priv = socket->priv;

    return FALSE;
}

static void
kiran_x11_tray_socket_class_init(KiranX11TraySocketClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    widget_class->realize = kiran_x11_tray_socket_realize;
    widget_class->draw = kiran_x11_tray_socket_draw;
}

static void
kiran_x11_tray_socket_init(KiranX11TraySocket *self)
{
    KiranX11TraySocketPrivate *priv;

    priv = self->priv = kiran_x11_tray_socket_get_instance_private(self);
    priv->has_alpha = TRUE;

    gtk_widget_set_hexpand(GTK_WIDGET(self), FALSE);
}

void kiran_x11_tray_socket_draw_on_parent(KiranX11TraySocket *socket,
                                          GtkWidget *parent,
                                          cairo_t *parent_cr)
{
    KiranX11TraySocketPrivate *priv;

    priv = socket->priv;

    if (priv->has_alpha)
    {
        GtkWidget *widget = GTK_WIDGET(socket);
        GtkAllocation parent_allocation = {0};
        GtkAllocation allocation;

        /* if the parent doesn't have a window, our allocation is not relative to
         * the context coordinates but to the parent's allocation */
        if (!gtk_widget_get_has_window(parent))
            gtk_widget_get_allocation(parent, &parent_allocation);

        gtk_widget_get_allocation(widget, &allocation);
        allocation.x -= parent_allocation.x;
        allocation.y -= parent_allocation.y;

        cairo_save(parent_cr);
        gdk_cairo_set_source_window(parent_cr,
                                    gtk_widget_get_window(widget),
                                    allocation.x,
                                    allocation.y);
        cairo_rectangle(parent_cr, allocation.x, allocation.y, allocation.width, allocation.height);
        cairo_clip(parent_cr);
        cairo_paint(parent_cr);
        cairo_restore(parent_cr);
    }
}

void kiran_x11_tray_socket_add_id(KiranX11TraySocket *socket,
                                  Window window,
                                  GdkScreen *screen)
{
    KiranX11TraySocketPrivate *priv = socket->priv;
    XWindowAttributes window_attributes;
    Display *xdisplay;
    GdkDisplay *display;
    GdkVisual *visual;
    gboolean visual_has_alpha;
    gint red_prec, green_prec, blue_prec, depth;
    gint result;

    xdisplay = GDK_SCREEN_XDISPLAY(screen);
    display = gdk_screen_get_display(screen);
    if (!GDK_IS_X11_DISPLAY(display))
        return;
    gdk_x11_display_error_trap_push(display);

    result = XGetWindowAttributes(xdisplay, window,
                                  &window_attributes);
    gdk_x11_display_error_trap_pop_ignored(display);

    if (!result)
        return;

    visual = gdk_x11_screen_lookup_visual(screen,
                                          window_attributes.visual->visualid);
    if (!visual)
        return;

    gtk_widget_set_visual(GTK_WIDGET(socket), visual);
    gdk_visual_get_red_pixel_details(visual, NULL, NULL, &red_prec);
    gdk_visual_get_green_pixel_details(visual, NULL, NULL, &green_prec);
    gdk_visual_get_blue_pixel_details(visual, NULL, NULL, &blue_prec);
    depth = gdk_visual_get_depth(visual);
    visual_has_alpha = red_prec + blue_prec + green_prec < depth;
    priv->has_alpha = (visual_has_alpha &&
                       gdk_display_supports_composite(gdk_screen_get_display(screen)));

    gtk_socket_add_id(GTK_SOCKET(socket), window);
}

GtkWidget *
kiran_x11_tray_socket_new(void)
{
    return g_object_new(KIRAN_TYPE_X11_TRAY_SOCKET, NULL);
}
