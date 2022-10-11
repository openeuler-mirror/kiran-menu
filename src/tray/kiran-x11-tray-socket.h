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

#ifndef __KIRAN_X11_TRAY_SOCKET__
#define __KIRAN_X11_TRAY_SOCKET__

#include <gtk/gtk.h>
#include <gtk/gtkx.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_X11_TRAY_SOCKET (kiran_x11_tray_socket_get_type())
#define KIRAN_X11_TRAY_SOCKET(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_X11_TRAY_SOCKET, KiranX11TraySocket))

typedef struct _KiranX11TraySocket KiranX11TraySocket;
typedef struct _KiranX11TraySocketClass KiranX11TraySocketClass;
typedef struct _KiranX11TraySocketPrivate KiranX11TraySocketPrivate;

struct _KiranX11TraySocket
{
    GtkSocket parent;

    KiranX11TraySocketPrivate *priv;
};

struct _KiranX11TraySocketClass
{
    GtkSocketClass parent_class;
};

GType kiran_x11_tray_socket_get_type(void);

GtkWidget *kiran_x11_tray_socket_new(void);
void kiran_x11_tray_socket_draw_on_parent(KiranX11TraySocket *socket,
                                          GtkWidget *parent,
                                          cairo_t *parent_cr);
void kiran_x11_tray_socket_add_id(KiranX11TraySocket *socket,
                                  Window window,
                                  GdkScreen *screen);

G_END_DECLS

#endif /* __KIRAN_X11_TRAY_SOCKET__ */
