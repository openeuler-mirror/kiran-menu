/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 21:00:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 00:58:46
 * @Description  : 开始菜单类，继承KiranMenuBased接口类。
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-skeleton.h
 */
#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-menu-based.h"

#define KIRAN_TYPE_MENU_SKELETON kiran_menu_skeleton_get_type()

G_DECLARE_FINAL_TYPE(KiranMenuSkeleton, kiran_menu_skeleton, KIRAN,
                     MENU_SKELETON, GObject)
