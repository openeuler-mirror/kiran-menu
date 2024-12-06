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
#include <xcb/xcb_image.h>

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

void resizeWindow(xcb_connection_t *c, xcb_window_t xcb_window_id)
{
    uint16_t widthNormalized = 18;
    uint16_t heighNormalized = 18;

    const uint32_t windowSizeConfigVals[2] = {widthNormalized, heighNormalized};
    xcb_configure_window(c, xcb_window_id, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, windowSizeConfigVals);

    xcb_flush(c);
}

void calculateClientWindowSize(xcb_connection_t *c, xcb_window_t xcb_window_id, int *w, int *h)
{
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(c, xcb_window_id);
    xcb_get_geometry_reply_t *clientGeom = xcb_get_geometry_reply(c, cookie, NULL);

    if (clientGeom)
    {
        *w = clientGeom->width;
        *h = clientGeom->height;
        free(clientGeom);
    }


    // if the window is a clearly stupid size resize to be something sensible
    // this is needed as chromium and such when resized just fill the icon with transparent space and only draw in the middle
    // however KeePass2 does need this as by default the window size is 273px wide and is not transparent
    // use an arbitrary heuristic to make sure icons are always sensible
    if (*w <= 0 || *h <= 0)
    {
        resizeWindow(c, xcb_window_id);
        *w = 18;
        *h = 18;
    }
}

void detect_non_blank_region(xcb_image_t *image, int *left, int *top, int *right, int *bottom)
{
    int width = image->width;
    int height = image->height;
    int stride = image->stride;
    int pixel_size = image->bpp / 8;

    *left = width;
    *right = 0;
    *top = height;
    *bottom = 0;

    // 临时变量，用于存储逐行检测的结果
    Bool found_non_blank_row = False;

    // 从上到下检测非空行
    for (int y = 0; y < height; y++)
    {
        const unsigned char *row = image->data + y * stride;
        unsigned int row_sum = 0;

        // 整行检测是否全为空
        for (int x = 0; x < width; x++)
        {
            const unsigned int *pixel = (const unsigned int *)(row + x * pixel_size);
            row_sum |= *pixel;
        }

        if (row_sum != 0)
        {  // 行非空
            if (!found_non_blank_row)
            {
                *top = y;  // 找到第一个非空行
                found_non_blank_row = True;
            }
            *bottom = y;  // 更新最后一个非空行
        }
    }

    // 如果没有找到任何非空行，说明图像完全空白
    if (*top == height)
    {
        *left = *top = *right = *bottom = 0;
        return;
    }

    // 从左到右检测列
    for (int x = 0; x < width; x++)
    {
        Bool column_non_blank = False;
        for (int y = *top; y <= *bottom; y++)
        {
            const unsigned char *row = image->data + y * stride;
            const unsigned int *pixel = (const unsigned int *)(row + x * pixel_size);

            if (*pixel != 0)
            {  // 列中有非空像素
                column_non_blank = True;
                break;
            }
        }

        if (column_non_blank)
        {
            if (x < *left) *left = x;
            if (x > *right) *right = x;
        }
    }
}

void kiran_x11_tray_socket_draw_on_parent(KiranX11TraySocket *socket,
                                          GtkWidget *parent,
                                          cairo_t *cr)
{
    auto xdisplay = gdk_x11_get_default_xdisplay();
    xcb_connection_t *c = XGetXCBConnection(xdisplay);

    GdkWindow *window = gtk_widget_get_window(socket);
    xcb_window_t xcb_window_id = GDK_WINDOW_XID(window);

    int w = 0;
    int h = 0;
    calculateClientWindowSize(c, xcb_window_id, &w, &h);
    xcb_image_t *image = xcb_image_get(c, xcb_window_id, 0, 0, w, h, ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);

    if (!image)
        return;

    // 图像属性
    // 使用图像深度推出Cairo格式
    cairo_format_t format;
    switch (image->bpp)
    {
    case 32:
        format = CAIRO_FORMAT_ARGB32;
        break;
    case 24:
        format = CAIRO_FORMAT_RGB24;
        break;
    default:
        g_print("Unsupported image format with %d bits per pixel\n", image->bpp);
        xcb_image_destroy(image);
        return;
    }

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        image->data,
        format,
        image->width,
        image->height,
        image->stride);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    {
        g_print("Failed to create Cairo image surface\n");
        xcb_image_destroy(image);
        return;
    }

    // 检测非空白区域
    int left, top, right, bottom;
    detect_non_blank_region(image, &left, &top, &right, &bottom);

    int new_width = (right - left + 1) / 2 * 2;
    int new_height = (bottom - top + 1) / 2 * 2;

    // 缩放并居中绘制
    // 固定绘制为18×18(panel最小为18)
    double target_width = 18;
    double target_height = 18;

    // 计算绘制时实际图像和需要绘制大小之间的缩放率
    double scale_x = target_width / new_width;
    double scale_y = target_height / new_height;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;

    // 计算窗口绘制起始坐标
    GtkAllocation allocation;
    gtk_widget_get_allocation(socket, &allocation);
    int widget_width = allocation.width / 2 * 2;  // 保证偶数，处理缩放模糊的情况
    int widget_height = allocation.height / 2 * 2;
    double offset_x = (widget_width - new_width * scale) / 2.0;
    double offset_y = (widget_height - new_height * scale) / 2.0;

    cairo_save(cr);
    cairo_translate(cr, offset_x, offset_y);  // 改变绘制起始坐标
    cairo_scale(cr, scale, scale);
    cairo_set_source_surface(cr, surface, -left, -top);
    cairo_paint(cr);
    cairo_restore(cr);

    // 清理
    cairo_surface_destroy(surface);
    xcb_image_destroy(image);
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
