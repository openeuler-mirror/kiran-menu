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

#ifndef MENU_POWER_BUTTON_H
#define MENU_POWER_BUTTON_H

#include <gtkmm.h>
#include "config.h"
#include "menu-power-menu.h"

class MenuPowerButton : public Gtk::Button
{
public:
    MenuPowerButton();
    ~MenuPowerButton();
    sigc::signal<void> signal_menu_hide();

protected:
    virtual void on_clicked() override;
    sigc::signal<void> m_signal_menu_hide;

private:
    Gtk::Image icon;
    Gtk::Menu *menu;
#ifdef POWER_DIALOG
    Gtk::Window *dialog_;
#endif
};

#endif  // MENU_POWER_BUTTON_H
