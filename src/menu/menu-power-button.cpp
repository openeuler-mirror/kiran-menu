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

#include "menu-power-button.h"
#include <glibmm/i18n.h>
#include "kiran-helper.h"

MenuPowerButton::MenuPowerButton() : menu(nullptr)
{
    auto context = get_style_context();

    icon.set_from_icon_name("kiran-menu-power-symbolic", Gtk::ICON_SIZE_BUTTON);
    add(icon);

    set_tooltip_text(_("Power options"));
    set_focus_on_click(false);

    context->add_class("menu-app-launcher");
    context->add_class("flat");
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
