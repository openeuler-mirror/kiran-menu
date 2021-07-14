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
