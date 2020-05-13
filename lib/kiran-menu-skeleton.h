/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 21:00:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 13:35:56
 * @Description  : 开始菜单类，继承KiranMenuBased接口类。
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-skeleton.h
 */
#pragma once

#include <gio/gio.h>

#include "lib/kiran-menu-based.h"
#include "lib/kiran-menu-unit.h"

#define KIRAN_TYPE_MENU_SKELETON kiran_menu_skeleton_get_type()

G_DECLARE_FINAL_TYPE(KiranMenuSkeleton, kiran_menu_skeleton, KIRAN,
                     MENU_SKELETON, GObject)

typedef enum
{
    KIRAN_MENU_TYPE_CATEGORY,
    KIRAN_MENU_TYPE_FAVORITE,
    KIRAN_MENU_TYPE_SEARCH,
    KIRAN_MENU_TYPE_SYSTEM,
    KIRAN_MENU_TYPE_USAGE,
} KiranMenuUnitType;

KiranMenuUnit *kiran_menu_skeleton_get_unit(KiranMenuSkeleton *self, KiranMenuUnitType unit_type);