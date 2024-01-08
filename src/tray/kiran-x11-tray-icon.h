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

#ifndef __KIRAN_X11_TRAY_ICON_H__
#define __KIRAN_X11_TRAY_ICON_H__
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_X11_TRAY_ICON (kiran_x11_tray_icon_get_type())
#define KIRAN_X11_TRAY_ICON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_X11_TRAY_ICON, KiranX11TrayIcon))
#define KIRAN_X11_TRAY_ICON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_X11_TRAY_ICON, KiranX11TrayIconClass))
#define KIRAN_IS_X11_TRAY_ICON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_X11_TRAY_ICON))
#define KIRAN_IS_X11_TRAY_ICON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_X11_TRAY_ICON))
#define KIRAN_X11_TRAY_CION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_X11_TRAY_ICON, KiranX11TrayIconClass))

typedef struct _KiranX11TrayIcon KiranX11TrayIcon;
typedef struct _KiranX11TrayIconClass KiranX11TrayIconClass;
typedef struct _KiranX11TrayIconPrivate KiranX11TrayIconPrivate;

struct _KiranX11TrayIcon
{
    GtkEventBox parent;

    KiranX11TrayIconPrivate *priv;
};

struct _KiranX11TrayIconClass
{
    GtkEventBoxClass parent_class;
};

GType kiran_x11_tray_icon_get_type(void);
GtkWidget *kiran_x11_tray_icon_new(Window icon_window);
GtkWidget *kiran_x11_tray_icon_get_socket(KiranX11TrayIcon *icon);
Window kiran_x11_tray_icon_get_icon_window(KiranX11TrayIcon *icon);

G_END_DECLS

#endif /* __KIRAN_X11_TRAY_ICON_H__ */
