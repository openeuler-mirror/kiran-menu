/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

#ifndef MENU_POWER_MENU_H
#define MENU_POWER_MENU_H

#include <gtkmm.h>
#include "kiran-opacity-menu.h"

class MenuPowerMenu : public KiranOpacityMenu
{
public:
    MenuPowerMenu();
    ~MenuPowerMenu() = default;

    void hide_menu_window();

private:
    Gtk::MenuItem *suspend_item;
    Gtk::MenuItem *hibernate_item;
    Gtk::MenuItem *shutdown_item;
    Gtk::MenuItem *reboot_item;
    Gtk::MenuItem *logout_item;
    Gtk::MenuItem *switchuser_item;
};

#endif  // MENU_POWER_MENU_H
