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

#include "menu-power-button.h"
#include <glibmm/i18n.h>
#include "kiran-helper.h"

MenuPowerButton::MenuPowerButton() : menu(nullptr)
{
    auto context = get_style_context();

    icon.set_from_resource("/kiran-menu/sidebar/power");
    add(icon);

    set_tooltip_text(_("Power options"));
    set_focus_on_click(false);

    context->add_class("menu-app-launcher");
}

MenuPowerButton::~MenuPowerButton()
{
    delete menu;
}

void MenuPowerButton::on_clicked()
{
    GdkEvent *event = gtk_get_current_event();

    if (menu)
        delete menu;

    menu = new MenuPowerMenu();
    menu->attach_to_widget(*this);
    menu->show_all();
    menu->popup_at_widget(this, Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_SOUTH_WEST, event);

    gdk_event_free(event);
}
