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
 * Author:     huangjiawen <huangjiawen@kylinos.com.cn>
 */

#include "menu-power-dialog.h"
#include <glibmm/i18n.h>
#include <gtk3-log-i.h>
#include <gtkmm/menu.h>
#include <iostream>
#include "global.h"
#include "kiran-helper.h"
#include "kiran-power.h"

MenuPowerDialog::MenuPowerDialog()
{
    Gtk::HeaderBar *dialog_header_bar;
    Gtk::Frame *dialog_frame;
    Gtk::Box *dialog_vbox;
    Gtk::Box *dialog_hbox;
    Gtk::Grid *dialog_grid;

    dialog_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
    dialog_header_bar->set_decoration_layout("menu,icon:close");
    dialog_header_bar->set_show_close_button();
    set_titlebar(*dialog_header_bar);

    set_modal();
    set_title(_("Shutdown"));
    set_border_width(5);
    set_icon_name("kiran-power-shutdown");
    set_position(Gtk::WIN_POS_CENTER);

    dialog_frame = Gtk::make_managed<Gtk::Frame>();
    dialog_frame->set_shadow_type(Gtk::SHADOW_OUT);
    add(*dialog_frame);

    Gdk::RGBA color;
    color.set_rgba(0.20, 0.20, 0.20, 1);
    dialog_header_bar->override_background_color(color, Gtk::STATE_FLAG_NORMAL);
    dialog_frame->override_background_color(color, Gtk::STATE_FLAG_NORMAL);

    dialog_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
    dialog_vbox->set_border_width(20);
    dialog_frame->add(*dialog_vbox);

    dialog_hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
    dialog_hbox->set_border_width(35);
    dialog_vbox->pack_start(*dialog_hbox, Gtk::PACK_EXPAND_PADDING);

    dialog_grid = Gtk::make_managed<Gtk::Grid>();
    dialog_grid->set_row_spacing(5);
    dialog_grid->set_column_spacing(10);
    dialog_hbox->pack_start(*dialog_grid, Gtk::PACK_EXPAND_PADDING);

    add_power_button(dialog_grid);

    get_style_context()->add_class("power-menu-dialog");
}

void MenuPowerDialog::add_power_button(Gtk::Grid *dialog_grid)
{
    gint space = 0;
    auto power = KiranPower::get_default();

    if (power->can_shutdown())
    {
        space += 1;
        add_button("kiran-power-shutdown", "Shutdown", dialog_grid, space, &MenuPowerDialog::do_shutdown);
    }

    if (power->can_reboot())
    {
        space += 1;
        add_button("kiran-power-reboot", "Reboot", dialog_grid, space, &MenuPowerDialog::do_reboot);
    }

    if (power->can_logout())
    {
        space += 1;
        add_button("kiran-power-logout", "Logout", dialog_grid, space, &MenuPowerDialog::do_logout);
    }

    if (power->can_hibernate())
    {
        space += 1;
        add_button("kiran-power-hibernate", "Hibernate", dialog_grid, space, &MenuPowerDialog::do_hibernate);
    }

    if (power->can_lock_screen())
    {
        space += 1;
        add_button("kiran-power-lock_screen", "Lock screen", dialog_grid, space, &MenuPowerDialog::do_lock_screen);
    }

    if (power->can_switch_user())
    {
        space += 1;
        add_button("kiran-power-switch_user", "Switch user", dialog_grid, space, &MenuPowerDialog::do_switch_user);
    }

    if (power->can_suspend())
    {
        space += 1;
        add_button("kiran-power-suspend", "Suspend", dialog_grid, space, &MenuPowerDialog::do_suspend);
    }
}

MenuPowerDialog::~MenuPowerDialog()
{
}

void MenuPowerDialog::add_button(const char *button_icon_name,
                                 const char *button_name,
                                 Gtk::Grid *dialog_grid,
                                 int space,
                                 void (MenuPowerDialog::*power_func)())
{
    Gtk::Button *power_button;
    Gtk::Label *power_label;

    power_button = Gtk::make_managed<Gtk::Button>();
    power_button->set_size_request(60, 40);
    power_button->set_image_from_icon_name(button_icon_name, Gtk::ICON_SIZE_DND);
    power_button->set_relief(Gtk::RELIEF_NONE);
    power_button->signal_clicked().connect(sigc::mem_fun(*this, power_func));

    dialog_grid->attach(*power_button, space, 0, 1, 1);

    power_label = Gtk::make_managed<Gtk::Label>(_(button_name));
    dialog_grid->attach(*power_label, space, 1, 1, 1);
}

void MenuPowerDialog::do_reboot()
{
    auto power = KiranPower::get_default();

    power->reboot();

    return;
}

void MenuPowerDialog::do_logout()
{
    auto power = KiranPower::get_default();

    power->logout(LOGOUT_MODE_NOW);

    return;
}

void MenuPowerDialog::do_shutdown()
{
    auto power = KiranPower::get_default();

    power->shutdown();

    return;
}

void MenuPowerDialog::do_hibernate()
{
    auto power = KiranPower::get_default();

    power->hibernate();

    return;
}
void MenuPowerDialog::do_suspend()
{
    auto power = KiranPower::get_default();

    power->suspend();

    return;
}

void MenuPowerDialog::do_switch_user()
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

    return;
}

void MenuPowerDialog::do_lock_screen()
{
    auto power = KiranPower::get_default();

    power->lock_screen();

    return;
}
