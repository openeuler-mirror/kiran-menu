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
