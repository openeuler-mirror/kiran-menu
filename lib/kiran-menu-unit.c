/*
 * @Author       : tangjie02
 * @Date         : 2020-05-11 11:20:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 13:47:02
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-unit.c
 */

#include "lib/kiran-menu-unit.h"

typedef struct
{
    gint type;
} KiranMenuUnitPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(KiranMenuUnit, kiran_menu_unit, G_TYPE_OBJECT);

void kiran_menu_unit_flush(KiranMenuUnit *unit, gpointer user_data)
{
    KiranMenuUnitClass *unit_class = KIRAN_MENU_UNIT_GET_CLASS(unit);
    if (unit_class->flush)
    {
        unit_class->flush(unit, user_data);
    }
}

static void kiran_menu_unit_dispose(GObject *object)
{
    // KiranMenuUnit *unit = GS_PAGE(object);
    // KiranMenUnitPrivate *priv = gs_page_get_instance_private(unit);

    G_OBJECT_CLASS(kiran_menu_unit_parent_class)->dispose(object);
}

static void kiran_menu_unit_init(KiranMenuUnit *unit)
{
}

static void kiran_menu_unit_class_init(KiranMenuUnitClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = kiran_menu_unit_dispose;
}