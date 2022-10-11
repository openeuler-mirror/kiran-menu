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

#include "kiran-notify-icon.h"

G_DEFINE_INTERFACE_WITH_CODE(KiranNotifyIcon, kiran_notify_icon, GTK_TYPE_WIDGET,
                             g_type_interface_add_prerequisite(g_define_type_id,
                                                               GTK_TYPE_ORIENTABLE);)

static void
kiran_notify_icon_default_init(KiranNotifyIconInterface *iface)
{
}

const gchar *
kiran_notify_icon_get_id(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), NULL);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_id != NULL, NULL);

    return iface->get_id(icon);
}

const gchar *
kiran_notify_icon_get_name(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), NULL);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_name != NULL, NULL);

    return iface->get_name(icon);
}

const gchar *
kiran_notify_icon_get_icon(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), NULL);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_icon != NULL, NULL);

    return iface->get_icon(icon);
}

KiranNotifyIconCategory
kiran_notify_icon_get_category(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_category != NULL, KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS);

    return iface->get_category(icon);
}

const gchar *
kiran_notify_icon_get_app_category(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), NULL);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_category != NULL, NULL);

    return iface->get_app_category(icon);
}

KiranNotifyIconWay
kiran_notify_icon_get_way(KiranNotifyIcon *icon)
{
    KiranNotifyIconInterface *iface;

    g_return_val_if_fail(KIRAN_IS_NOTIFY_ICON(icon), KIRAN_NOTIFY_ICON_WAY_OTHER);

    iface = KIRAN_NOTIFY_ICON_GET_IFACE(icon);
    g_return_val_if_fail(iface->get_way != NULL, KIRAN_NOTIFY_ICON_WAY_OTHER);

    return iface->get_way(icon);
}

gint kiran_notify_icon_compare(KiranNotifyIcon *icon_a,
                               KiranNotifyIcon *icon_b)
{
    KiranNotifyIconCategory category_a;
    KiranNotifyIconCategory category_b;
    const char *id_a;
    const char *id_b;

    if (!KIRAN_IS_NOTIFY_ICON(icon_a))
        return -1;

    if (!KIRAN_IS_NOTIFY_ICON(icon_b))
        return +1;

    category_a = kiran_notify_icon_get_category(icon_a);
    category_b = kiran_notify_icon_get_category(icon_b);
    id_a = kiran_notify_icon_get_id(icon_a);
    id_b = kiran_notify_icon_get_id(icon_b);

    if (category_a == category_b)
        return g_strcmp0(id_a, id_b);

    if (category_a < category_b)
        return -1;

    return +1;
}
