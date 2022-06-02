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
#include <gtk3-log-i.h>
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
        this->switchuser_item_ = Gtk::make_managed<Gtk::MenuItem>(_("Switch user"));
        this->switchuser_item_->signal_activate().connect(sigc::mem_fun(this, &MenuPowerMenu::on_switch_user_cb));

        append(*this->switchuser_item_);
    }

    if (power->can_logout())
    {
        this->logout_item_ = Gtk::make_managed<Gtk::MenuItem>(_("Logout"));
        this->logout_item_->signal_activate().connect([power]() {
            power->logout(LOGOUT_MODE_NOW);
        });
        append(*this->logout_item_);
    }

    if (power->can_suspend())
    {
        this->suspend_item_ = Gtk::make_managed<Gtk::MenuItem>(_("Suspend"));
        this->suspend_item_->signal_activate().connect([power]() {
            power->suspend();
        });
        append(*this->suspend_item_);
    }

    if (power->can_hibernate())
    {
        this->hibernate_item_ = Gtk::make_managed<Gtk::MenuItem>(_("Hibernate"));
        this->hibernate_item_->signal_activate().connect([power]() {
            power->hibernate();
        });

        append(*this->hibernate_item_);
    }

    if (power->can_reboot())
    {
        this->reboot_item = Gtk::make_managed<Gtk::MenuItem>(_("Reboot"));
        this->reboot_item->signal_activate().connect([power]() {
            power->reboot();
        });
        append(*this->reboot_item);
    }

    if (power->can_shutdown())
    {
        this->shutdown_item_ = Gtk::make_managed<Gtk::MenuItem>(_("Shutdown"));
        this->shutdown_item_->signal_activate().connect([power]() {
            power->shutdown();
        });
        append(*this->shutdown_item_);
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

void MenuPowerMenu::on_switch_user_cb()
{
    auto power = KiranPower::get_default();

    if (power->get_graphical_ntvs() >= power->get_ntvs_total())
    {
        KLOG_DEBUG("Total ntvs: %d, graphical ntvs: %d.", power->get_ntvs_total(), power->get_graphical_ntvs());

        Gtk::MessageDialog dialog(_("The logined users reach the maximum limit at the same time, "
                                    "you cannot switch to the greeter interface, Please log off some logined users first."),
                                  true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        dialog.set_title(_("System Warning"));
        dialog.run();
    }
    else
    {
        power->switch_user();
    }
}
