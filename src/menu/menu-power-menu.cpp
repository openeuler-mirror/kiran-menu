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

#include "menu-power-menu.h"
#include <glibmm/i18n.h>
#include <iostream>
#include "global.h"
#include "kiran-helper.h"
#include "kiran-power.h"

MenuPowerMenu::MenuPowerMenu()
{
    auto power = KiranPower::get_default();

    if (power->can_lock_screen())
    {
        auto lock_item = Gtk::make_managed<Gtk::MenuItem>(_("Lock screen"));
        lock_item->signal_activate().connect([power]() {
            power->lock_screen();
        });

        append(*lock_item);
    }

    if (power->can_switch_user())
    {
        switchuser_item = Gtk::make_managed<Gtk::MenuItem>(_("Switch user"));
        switchuser_item->signal_activate().connect([power]() {
            power->switch_user();
        });

        append(*switchuser_item);
    }

    if (power->can_logout())
    {
        logout_item = Gtk::make_managed<Gtk::MenuItem>(_("Logout"));
        logout_item->signal_activate().connect([power]() {
            power->logout(LOGOUT_MODE_NOW);
        });
        append(*logout_item);
    }

    if (power->can_suspend())
    {
        suspend_item = Gtk::make_managed<Gtk::MenuItem>(_("Suspend"));
        suspend_item->signal_activate().connect([power]() {
            power->suspend();
        });
        append(*suspend_item);
    }

    if (power->can_hibernate())
    {
        hibernate_item = Gtk::make_managed<Gtk::MenuItem>(_("Hibernate"));
        hibernate_item->signal_activate().connect([power]() {
            power->hibernate();
        });

        append(*hibernate_item);
    }

    if (power->can_reboot())
    {
        reboot_item = Gtk::make_managed<Gtk::MenuItem>(_("Reboot"));
        reboot_item->signal_activate().connect([power]() {
            power->reboot();
        });
        append(*reboot_item);
    }

    if (power->can_shutdown())
    {
        shutdown_item = Gtk::make_managed<Gtk::MenuItem>(_("Shutdown"));
        shutdown_item->signal_activate().connect([power]() {
            power->shutdown();
        });
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
