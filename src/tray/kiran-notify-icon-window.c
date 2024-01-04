/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "kiran-notify-icon-window.h"

#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <math.h>

struct _KiranNotifyIconWindowPrivate
{
    GtkWidget *grid;
    gint icon_count;
    Window prev_active_window;  //前一时刻活动窗口
    GtkWidget *select;
};

static GObject *kiran_notify_icon_window_constructor(GType type,
                                                     guint n_construct_properties,
                                                     GObjectConstructParam *construct_properties);

G_DEFINE_TYPE_WITH_PRIVATE(KiranNotifyIconWindow, kiran_notify_icon_window, GTK_TYPE_WINDOW)

static gboolean
kiran_notify_icon_window_grab(GtkWidget *widget)
{
    GdkGrabStatus status;
    GdkDisplay *display;
    GdkSeat *seat;

    if (!gtk_widget_is_visible(widget))
        return FALSE;

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);

    status = gdk_seat_grab(seat, gtk_widget_get_window(widget),
                           GDK_SEAT_CAPABILITY_POINTER, TRUE,
                           NULL, NULL, NULL, NULL);
    return status == GDK_GRAB_SUCCESS;
}

static void
kiran_notify_icon_window_ungrab(GtkWidget *widget)
{
    GdkDisplay *display;
    GdkSeat *seat;

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);

    gdk_seat_ungrab(seat);
}

static gboolean
kiran_notify_icon_window_map_event(GtkWidget *widget,
                                   GdkEventAny *event)
{
    return kiran_notify_icon_window_grab(widget);
}

static gboolean
kiran_notify_icon_window_unmap_event(GtkWidget *widget,
                                     GdkEventAny *event)
{
    kiran_notify_icon_window_ungrab(widget);

    return FALSE;
}

static gboolean
kiran_notify_icon_window_button_press_event(GtkWidget *widget,
                                            GdkEventButton *event)
{
    GdkWindow *window;
    gint root_x, root_y;
    gint width, height;

    window = gtk_widget_get_window(widget);

    gdk_window_get_origin(window, &root_x, &root_y);
    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    if (event->x_root < root_x || event->x_root > root_x + width ||
        event->y_root < root_y || event->y_root > root_y + height)
    {
        gtk_widget_hide(widget);
    }

    return FALSE;
}

static gboolean
kiran_notify_icon_window_motion_notify_event(GtkWidget *widget,
                                             GdkEventMotion *event)
{
    GdkWindow *window;
    gint root_x, root_y;
    gint width, height;

    window = gtk_widget_get_window(widget);

    gdk_window_get_origin(window, &root_x, &root_y);
    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    if (event->x_root >= root_x && event->x_root <= root_x + width &&
        event->y_root >= root_y && event->y_root <= root_y + height)
    {
        //进入窗口后，释放抓取， 便于嵌入图标获得焦点
        kiran_notify_icon_window_ungrab(widget);
    }

    return FALSE;
}

static gboolean
kiran_notify_icon_window_leave_notify_event(GtkWidget *widget,
                                            GdkEventCrossing *event)
{
    GdkDisplay *display;
    GdkSeat *seat;
    GdkDevice *pointer;
    gint win_x, win_y;

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);
    pointer = gdk_seat_get_pointer(seat);

    //鼠标离开窗口后，获取当前的鼠标的window为空
    if (!gdk_device_get_window_at_position(pointer, &win_x, &win_y))
    {
        kiran_notify_icon_window_grab(widget);
    }

    return FALSE;
}

static gboolean
kiran_notify_icon_window_focus_in_event(GtkWidget *widget,
                                        GdkEventFocus *event)
{
    KiranNotifyIconWindow *win;
    KiranNotifyIconWindowPrivate *priv;
    GdkWindow *window;
    GdkDisplay *display;
    GdkSeat *seat;
    GdkDevice *pointer;
    gint pointer_x, pointer_y;
    gint root_x, root_y;
    gint width, height;

    window = gtk_widget_get_window(widget);

    win = KIRAN_NOTIFY_ICON_WINDOW(widget);
    priv = win->priv;

    gdk_window_get_origin(window, &root_x, &root_y);
    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);
    pointer = gdk_seat_get_pointer(seat);
    gdk_device_get_position(pointer, NULL, &pointer_x, &pointer_y);

    if (pointer_x < root_x || pointer_x > root_x + width ||
        pointer_y < root_y || pointer_y > root_y + height)
    {
        kiran_notify_icon_window_grab(widget);
    }

    //恢复之前的选中状态
    if (priv->select)
    {
        GList *children;
        GList *child;

        children = gtk_container_get_children(GTK_CONTAINER(priv->grid));
        for (child = children; child; child = child->next)
        {
            if (priv->select == child->data)
            {
                gtk_widget_set_state_flags(priv->select, GTK_STATE_FLAG_PRELIGHT, FALSE);
                priv->select = NULL;
                break;
            }
        }
        g_list_free(children);
    }

    return FALSE;
}

static gboolean
kiran_notify_icon_window_focus_out_event(GtkWidget *widget,
                                         GdkEventFocus *event)
{
    KiranNotifyIconWindow *win;
    KiranNotifyIconWindowPrivate *priv;
    GList *children;
    GList *child;

    win = KIRAN_NOTIFY_ICON_WINDOW(widget);
    priv = win->priv;

    children = gtk_container_get_children(GTK_CONTAINER(priv->grid));

    for (child = children; child; child = child->next)
    {
        GtkStateFlags flags;

        flags = gtk_widget_get_state_flags(child->data);

        //取消选中状态
        if (flags & GTK_STATE_FLAG_PRELIGHT)
        {
            flags = flags & ~GTK_STATE_FLAG_PRELIGHT;
            gtk_widget_set_state_flags(child->data, flags, TRUE);
            priv->select = child->data;
            break;
        }
    }
    g_list_free(children);

    return FALSE;
}

static gboolean
kiran_notify_icon_window_draw(GtkWidget *widget,
                              cairo_t *cr)
{
    GtkAllocation alloc;
    gboolean ret = FALSE;
    gdouble degrees = M_PI / 180.0;
    gint radius = 6;

    gtk_widget_get_allocation(widget, &alloc);

    cairo_new_sub_path(cr);
    cairo_arc(cr, alloc.x + alloc.width - radius, alloc.y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, alloc.x + alloc.width - radius, alloc.y + alloc.height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, alloc.x + radius, alloc.y + alloc.height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, alloc.x + radius, alloc.y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);

    cairo_clip(cr);

    if (GTK_WIDGET_CLASS(kiran_notify_icon_window_parent_class)->draw)
        ret = GTK_WIDGET_CLASS(kiran_notify_icon_window_parent_class)->draw(widget, cr);

    return ret;
}

static void
kiran_notify_icon_window_class_init(KiranNotifyIconWindowClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    gobject_class->constructor = kiran_notify_icon_window_constructor;
    widget_class->map_event = kiran_notify_icon_window_map_event;
    widget_class->unmap_event = kiran_notify_icon_window_unmap_event;
    widget_class->button_press_event = kiran_notify_icon_window_button_press_event;
    widget_class->motion_notify_event = kiran_notify_icon_window_motion_notify_event;
    widget_class->leave_notify_event = kiran_notify_icon_window_leave_notify_event;
    widget_class->focus_in_event = kiran_notify_icon_window_focus_in_event;
    widget_class->focus_out_event = kiran_notify_icon_window_focus_out_event;
    widget_class->draw = kiran_notify_icon_window_draw;
}

static void
kiran_notify_icon_window_init(KiranNotifyIconWindow *window)
{
    KiranNotifyIconWindowPrivate *priv;

    priv = window->priv = kiran_notify_icon_window_get_instance_private(window);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_stick(GTK_WINDOW(window));
    gtk_window_set_title(GTK_WINDOW(window), "Kiran Notify Icon");
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);

    priv->icon_count = 0;
    priv->prev_active_window = 0;
    priv->select = NULL;
}

static void tran_setup(GtkWidget *window)
{
    GdkScreen *screen;
    GdkVisual *visual;

    gtk_widget_set_app_paintable(window, TRUE);
    screen = gdk_screen_get_default();
    visual = gdk_screen_get_rgba_visual(screen);

    if (visual != NULL && gdk_screen_is_composited(screen))
    {
        gtk_widget_set_visual(window, visual);
    }
}

static GObject *
kiran_notify_icon_window_constructor(GType type,
                                     guint n_construct_properties,
                                     GObjectConstructParam *construct_properties)
{
    GObject *obj;
    KiranNotifyIconWindow *win;
    GtkWidget *box;

    obj = G_OBJECT_CLASS(kiran_notify_icon_window_parent_class)->constructor(type, n_construct_properties, construct_properties);
    win = KIRAN_NOTIFY_ICON_WINDOW(obj);
    tran_setup(GTK_WIDGET(win));
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(GTK_WIDGET(win), 1, 1);
    gtk_widget_set_name(GTK_WIDGET(box), "trayWindowBox");
    win->priv->grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(win->priv->grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(win->priv->grid), TRUE);
    gtk_widget_set_name(GTK_WIDGET(win->priv->grid), "trayGrid");

    gtk_box_pack_start(GTK_BOX(box), win->priv->grid, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(win), box);
    gtk_widget_show(win->priv->grid);
    gtk_widget_show(box);

    return obj;
}

static void
kiran_notify_icon_window_icons_refresh(KiranNotifyIconWindow *window,
                                       gint max_row)
{
    KiranNotifyIconWindowPrivate *priv = window->priv;
    GList *children;
    GList *child;
    gint index = 0;
    gint row = 0;
    gint col = 0;

    children = gtk_container_get_children(GTK_CONTAINER(priv->grid));
    children = g_list_sort(children, (GCompareFunc)kiran_notify_icon_compare);

    for (child = children; child; child = child->next)
    {
        if (max_row == 1)
        {
            row = 0;
            col = index;
        }
        else
        {
            row = index / max_row;
            col = index % max_row;
        }
        gtk_container_child_set(GTK_CONTAINER(priv->grid),
                                child->data,
                                "left-attach", col,
                                "top-attach", row,
                                NULL);
        index++;
    }

    if (max_row == 1)
    {
        if (col > 0)
            max_row = 2;
    }

    gtk_widget_set_size_request(GTK_WIDGET(window), max_row * 40, (row + 1) * 40);
    g_list_free(children);
}

static void
computer_max_row(gint count,
                 gint *row)
{
    gint i;

    for (i = 1;; i++)
    {
        if ((count / i < i) ||
            (count / i == i && (count % i == 0)))
            break;
    }

    *row = i;
}

void kiran_notify_icon_window_add_icon(KiranNotifyIconWindow *window,
                                       KiranNotifyIcon *icon)
{
    KiranNotifyIconWindowPrivate *priv = window->priv;
    gint row;

    gtk_grid_attach(GTK_GRID(priv->grid),
                    GTK_WIDGET(icon),
                    (priv->icon_count - 1), 0,
                    1, 1);
    priv->icon_count++;

    if (priv->icon_count <= 2)
    {
        kiran_notify_icon_window_icons_refresh(window, 1);
        return;
    }

    computer_max_row(priv->icon_count, &row);
    kiran_notify_icon_window_icons_refresh(window, row);
}

void kiran_notify_icon_window_remove_icon(KiranNotifyIconWindow *window,
                                          KiranNotifyIcon *icon)
{
    KiranNotifyIconWindowPrivate *priv = window->priv;
    gint row;

    gtk_container_remove(GTK_CONTAINER(priv->grid), GTK_WIDGET(icon));
    priv->icon_count--;

    if (priv->icon_count < 2)
    {
        kiran_notify_icon_window_icons_refresh(window, 1);
        return;
    }

    computer_max_row(priv->icon_count, &row);
    kiran_notify_icon_window_icons_refresh(window, row);
}

gint kiran_notify_icon_window_get_icons_number(KiranNotifyIconWindow *window)
{
    KiranNotifyIconWindowPrivate *priv = window->priv;

    return priv->icon_count;
}

GtkWidget *
kiran_notify_icon_window_new(void)
{
    return g_object_new(KIRAN_TYPE_NOTIFY_ICON_WINDOW, NULL);
}
