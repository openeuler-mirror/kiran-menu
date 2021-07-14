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

#include "menu-power-menu.h"
#include <glibmm/i18n.h>
#include <iostream>
#include "global.h"
#include "kiran-helper.h"
#include "kiran-power.h"

MenuPowerMenu::MenuPowerMenu()
{
    auto lock_item = Gtk::make_managed<Gtk::MenuItem>(_("Lock screen"));
    lock_item->signal_activate().connect(sigc::hide_return(
        sigc::ptr_fun(&KiranPower::lock_screen)));
    append(*lock_item);

    if (KiranPower::can_switchuser())
    {
        switchuser_item = Gtk::make_managed<Gtk::MenuItem>(_("Switch user"));
        switchuser_item->signal_activate().connect(sigc::hide_return(
            sigc::ptr_fun(&KiranPower::switch_user)));

        append(*switchuser_item);
    }

    logout_item = Gtk::make_managed<Gtk::MenuItem>(_("Logout"));
    logout_item->signal_activate().connect(sigc::hide_return(
        sigc::bind<int>(sigc::ptr_fun(&KiranPower::logout), LOGOUT_MODE_NOW)));
    append(*logout_item);

    if (KiranPower::can_suspend())
    {
        suspend_item = Gtk::make_managed<Gtk::MenuItem>(_("Suspend"));
        suspend_item->signal_activate().connect(sigc::hide_return(
            sigc::ptr_fun(&KiranPower::suspend)));

        append(*suspend_item);
    }

    if (KiranPower::can_hibernate())
    {
        hibernate_item = Gtk::make_managed<Gtk::MenuItem>(_("Hibernate"));
        hibernate_item->signal_activate().connect(sigc::hide_return(
            sigc::ptr_fun(&KiranPower::hibernate)));

        append(*hibernate_item);
    }

    if (KiranPower::can_reboot())
    {
        reboot_item = Gtk::make_managed<Gtk::MenuItem>(_("Reboot"));
        reboot_item->signal_activate().connect(
            sigc::hide_return(sigc::ptr_fun(&KiranPower::reboot)));
        append(*reboot_item);
    }

    // if (KiranPower::can_shutdown())
    {
        shutdown_item = Gtk::make_managed<Gtk::MenuItem>(_("Shutdown"));
        shutdown_item->signal_activate().connect(
            sigc::hide_return(sigc::ptr_fun(&KiranPower::shutdown)));
        append(*shutdown_item);
    }

    for (auto child : get_children())
    {
        Gtk::MenuItem *item = dynamic_cast<Gtk::MenuItem *>(child);
        //点击电源菜单选项后收起开始菜单窗口
        item->signal_activate().connect(
            sigc::mem_fun(*this, &MenuPowerMenu::hide_menu_window));
    }

    get_style_context()->add_class("power-menu");
}

void MenuPowerMenu::hide_menu_window()
{
    Gtk::Widget *menu_window;

    if (!get_attach_widget())
        return;
    menu_window = get_attach_widget()->get_toplevel();
    menu_window->hide();
}
