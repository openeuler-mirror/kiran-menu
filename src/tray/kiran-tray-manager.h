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

#ifndef __KIRAN_TRAY_MANAGER__
#define __KIRAN_TRAY_MANAGER__
#include <gtk/gtk.h>
#include "kiran-notify-icon.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_TRAY_MANAGER (kiran_tray_manager_get_type())
#define KIRAN_TRAY_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_TRAY_MANAGER, KiranTrayManager))
#define KIRAN_IS_TRAY_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_TRAY_MANAGER))
#define KIRAN_TRAY_MANAGER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), KIRAN_TYPE_TRAY_MANAGER, KiranTrayManagerInterface))

typedef struct _KiranTrayManager KiranTrayManager;
typedef struct _KiranTrayManagerInterface KiranTrayManagerInterface;

struct _KiranTrayManagerInterface
{
    GTypeInterface parent;

    void (*set_icon_size)(KiranTrayManager *manager,
                          gint icon_size);
};

GType kiran_tray_manager_get_type(void);
void kiran_tray_manager_emit_icon_added(KiranTrayManager *manager,
                                        KiranNotifyIcon *icon);
void kiran_tray_manager_emit_icon_removed(KiranTrayManager *manager,
                                          KiranNotifyIcon *icon);
void kiran_tray_manager_set_icon_size(KiranTrayManager *manager,
                                      gint icon_size);

G_END_DECLS

#endif /* __KIRAN_TRAY_MANAGER__ */
