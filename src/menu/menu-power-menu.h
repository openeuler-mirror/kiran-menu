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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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
    void on_switch_user_cb();

private:
    Gtk::MenuItem *suspend_item_;
    Gtk::MenuItem *hibernate_item_;
    Gtk::MenuItem *shutdown_item_;
    Gtk::MenuItem *reboot_item;
    Gtk::MenuItem *logout_item_;
    Gtk::MenuItem *switchuser_item_;
};

#endif  // MENU_POWER_MENU_H
