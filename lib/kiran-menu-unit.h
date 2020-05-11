/*
 * @Author       : tangjie02
 * @Date         : 2020-05-11 11:19:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 13:46:50
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-unit.h
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_UNIT (kiran_menu_unit_get_type())

G_DECLARE_DERIVABLE_TYPE(KiranMenuUnit, kiran_menu_unit, KIRAN, MENU_UNIT, GObject);

struct _KiranMenuUnitClass
{
    GObjectClass parent_class;

    void (*flush)(KiranMenuUnit *unit, gpointer user_data);
};

void kiran_menu_unit_flush(KiranMenuUnit *unit, gpointer user_data);

G_END_DECLS