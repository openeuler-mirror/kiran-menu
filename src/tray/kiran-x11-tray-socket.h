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
