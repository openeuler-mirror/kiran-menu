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

#ifndef __KIRAN_SN_ICON_MENU_H__
#define __KIRAN_SN_ICON_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_SN_ICON_MENU (kiran_sn_icon_menu_get_type())
#define KIRAN_SN_ICON_MENU(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_SN_ICON_MENU, KiranSnIconMenu))
#define KIRAN_SN_ICON_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_SN_ICON_MENU, KiranSnIconMenuClass))
#define KIRAN_IS_SN_ICON_MENU(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_SN_ICON_MENU))
#define KIRAN_IS_SN_ICON_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_SN_ICON_MENU))
#define KIRAN_SN_ICON_MENU_GET_CLASS(obj) (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_SN_ICON_MENU, KiranSnIconMenuClass))

typedef struct _KiranSnIconMenu KiranSnIconMenu;
typedef struct _KiranSnIconMenuClass KiranSnIconMenuClass;
typedef struct _KiranSnIconMenuPrivate KiranSnIconMenuPrivate;

struct _KiranSnIconMenu
{
    GtkMenu parent;

    KiranSnIconMenuPrivate *priv;
};

struct _KiranSnIconMenuClass
{
    GtkMenuClass parent_class;
};

GType kiran_sn_icon_menu_get_type(void);

GtkMenu *kiran_sn_icon_menu_new(gchar *bus_name,
                                gchar *object_path);

G_END_DECLS

#endif /* __KIRAN_SN_ICON_MENU_H__ */
