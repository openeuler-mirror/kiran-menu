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

#ifndef __KIRAN_X11_TRAY_MANAGER__
#define __KIRAN_X11_TRAY_MANAGER__
#include <gtk/gtk.h>

#include "kiran-tray-manager.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_X11_TRAY_MANAGER (kiran_x11_tray_manager_get_type())
#define KIRAN_X11_TRAY_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_X11_TRAY_MANAGER, KiranX11TrayManager))
#define KIRAN_X11_TRAY_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_X11_TRAY_MANAGER, KiranX11TrayManagerClass))
#define KIRAN_IS_X11_TRAY_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_X11_TRAY_MANAGER))
#define KIRAN_IS_X11_TRAY_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_X11_TRAY_MANAGER))
#define KIRAN_X11_TRAY_MANAGER_GET_CLASS(obj) (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_X11_TRAY_MANAGER, KiranX11TrayManagerClass))

typedef struct _KiranX11TrayManager KiranX11TrayManager;
typedef struct _KiranX11TrayManagerClass KiranX11TrayManagerClass;
typedef struct _KiranX11TrayManagerPrivate KiranX11TrayManagerPrivate;

struct _KiranX11TrayManager
{
    GObject parent;

    KiranX11TrayManagerPrivate *priv;
};

struct _KiranX11TrayManagerClass
{
    GObjectClass parent_class;
};

GType kiran_x11_tray_manager_get_type(void);
KiranTrayManager *kiran_x11_tray_manager_new(GdkScreen *screen);

G_END_DECLS

#endif /* __KIRAN_X11_TRAY_MANAGER__ */
