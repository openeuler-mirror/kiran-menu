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

#ifndef MENU_POWER_BUTTON_H
#define MENU_POWER_BUTTON_H

#include <gtkmm.h>
#include "menu-power-menu.h"

class MenuPowerButton : public Gtk::Button
{
public:
    MenuPowerButton();
    ~MenuPowerButton();

protected:
    virtual void on_clicked() override;

private:
    Gtk::Image icon;
    Gtk::Menu *menu;
};

#endif  // MENU_POWER_BUTTON_H
