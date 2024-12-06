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

#ifndef __KIRAN_NOTIFY_ICON__
#define __KIRAN_NOTIFY_ICON__
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_NOTIFY_ICON (kiran_notify_icon_get_type())
#define KIRAN_NOTIFY_ICON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_NOTIFY_ICON, KiranNotifyIcon))
#define KIRAN_IS_NOTIFY_ICON(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_NOTIFY_ICON))
#define KIRAN_NOTIFY_ICON_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), KIRAN_TYPE_NOTIFY_ICON, KiranNotifyIconInterface))

typedef struct _KiranNotifyIcon KiranNotifyIcon;
typedef struct _KiranNotifyIconInterface KiranNotifyIconInterface;

typedef enum
{
    KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS,
    KIRAN_NOTIFY_ICON_CATEGORY_COMMUNICATIONS,
    KIRAN_NOTIFY_ICON_CATEGORY_SYSTEM_SERVICES,
    KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE
} KiranNotifyIconCategory;

static const struct
{
    const gchar *const wm_class;
    const gchar *const id;
    KiranNotifyIconCategory category;
} wmclass_categories[] = {
    {"keyboard", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Mate-volume-control-applet", "~02-volume", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Bluetooth-applet", "~03-bluetooth", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Nm-applet", "~04-network", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"kiran-network-status-icon", "~04-network", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Mate-power-manager", "~05-battery", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Kiran-power-status-icon", "~05-battery", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Mate-volume-control-status-icon", "~02-volume", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"kiran-audio-status-icon", "~02-volume", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"ibus-ui-gtk", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"Fcitx", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"fcitx", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"fcitx-qimpanel", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"scim", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"sogou-qimpanel", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
    {"baidu-qimpanel", "~01-keyboard", KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE},
};

typedef enum
{
    KIRAN_NOTIFY_ICON_WAY_X11,
    KIRAN_NOTIFY_ICON_WAY_DBUS,
    KIRAN_NOTIFY_ICON_WAY_OTHER
} KiranNotifyIconWay;

struct _KiranNotifyIconInterface
{
    GTypeInterface g_iface;

    const gchar *(*get_id)(KiranNotifyIcon *icon);
    const gchar *(*get_name)(KiranNotifyIcon *icon);
    const gchar *(*get_icon)(KiranNotifyIcon *icon);
    const gchar *(*get_app_category)(KiranNotifyIcon *icon);
    KiranNotifyIconCategory (*get_category)(KiranNotifyIcon *icon);
    KiranNotifyIconWay (*get_way)(KiranNotifyIcon *icon);
};

GType kiran_notify_icon_get_type(void);

const gchar *kiran_notify_icon_get_id(KiranNotifyIcon *icon);
const gchar *kiran_notify_icon_get_name(KiranNotifyIcon *icon);
const gchar *kiran_notify_icon_get_icon(KiranNotifyIcon *icon);
KiranNotifyIconCategory kiran_notify_icon_get_category(KiranNotifyIcon *icon);
const gchar *kiran_notify_icon_get_app_category(KiranNotifyIcon *icon);
KiranNotifyIconWay kiran_notify_icon_get_way(KiranNotifyIcon *icon);
gint kiran_notify_icon_compare(KiranNotifyIcon *icon_a,
                               KiranNotifyIcon *icon_b);

G_END_DECLS
#endif /* __KIRAN_NOTIFY_ICON__ */
