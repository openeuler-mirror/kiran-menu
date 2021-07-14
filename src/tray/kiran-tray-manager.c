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

#include "kiran-tray-manager.h"

enum
{
    SIGNAL_ICON_ADDED,
    SIGNAL_ICON_REMOVED,

    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_INTERFACE(KiranTrayManager, kiran_tray_manager, G_TYPE_OBJECT)

static void
kiran_tray_manager_default_init(KiranTrayManagerInterface *iface)
{
    signals[SIGNAL_ICON_ADDED] =
        g_signal_new("icon-added", G_TYPE_FROM_INTERFACE(iface),
                     G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                     G_TYPE_NONE, 1, KIRAN_TYPE_NOTIFY_ICON);

    signals[SIGNAL_ICON_REMOVED] =
        g_signal_new("icon-removed", G_TYPE_FROM_INTERFACE(iface),
                     G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                     G_TYPE_NONE, 1, KIRAN_TYPE_NOTIFY_ICON);
}
void kiran_tray_manager_emit_icon_added(KiranTrayManager *manager,
                                        KiranNotifyIcon *icon)
{
    g_signal_emit(manager, signals[SIGNAL_ICON_ADDED], 0, icon);
}

void kiran_tray_manager_emit_icon_removed(KiranTrayManager *manager,
                                          KiranNotifyIcon *icon)
{
    g_signal_emit(manager, signals[SIGNAL_ICON_REMOVED], 0, icon);
}

void kiran_tray_manager_set_icon_size(KiranTrayManager *manager,
                                      gint icon_size)
{
    KiranTrayManagerInterface *iface;

    g_return_if_fail(KIRAN_IS_TRAY_MANAGER(manager));

    iface = KIRAN_TRAY_MANAGER_GET_IFACE(manager);
    g_return_if_fail(iface->set_icon_size != NULL);

    iface->set_icon_size(manager,
                         icon_size);
}
