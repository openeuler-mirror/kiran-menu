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

#ifndef __KIRAN_TRAY_H__
#define __KIRAN_TRAY_H__

#include <gtk/gtk.h>

#define KIRAN_TRAY_SCHEMA "com.kylinsec.kiran.tray"
#define KEY_PANEL_ICON_IDS "panel-icon-ids"
#define KEY_HIDE_ICON_IDS "hide-icon-ids"
#define KEY_TRAY_ICON_SIZE "tray-icon-size"
#define KEY_TRAY_ICON_PADDING "panel-icon-padding"

#define ICON_SHOW_IN_PANEL 0
#define ICON_SHOW_IN_WINDOW 1
#define ICON_NOT_SHOW 2

G_BEGIN_DECLS

#define KIRAN_TYPE_TRAY (kiran_tray_get_type())
#define KIRAN_TRAY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_TRAY, KiranTray))
#define KIRAN_TRAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_TRAY, KiranTrayClass))
#define KIRAN_IS_TRAY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_TRAY))
#define KIRAN_IS_TRAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_TRAY))
#define KIRAN_TRAY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_TRAY, KiranTrayClass))

typedef enum
{
    APPLET_ORIENT_RIGHT,
    APPLET_ORIENT_LEFT,
    APPLET_ORIENT_DOWN,
    APPLET_ORIENT_UP
} KiranTrayLoaction;

typedef struct _KiranTray KiranTray;
typedef struct _KiranTrayClass KiranTrayClass;

typedef struct _KiranTrayPrivate KiranTrayPrivate;

struct _KiranTray
{
    GtkBox parent;

    KiranTrayPrivate *priv;
};

struct _KiranTrayClass
{
    GtkBoxClass parent_class;
};

G_END_DECLS

GType kiran_tray_get_type(void);

GtkWidget *kiran_tray_new(void);

#endif /* __KIRAN_TRAY_H__ */
