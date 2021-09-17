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
