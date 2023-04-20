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

#ifndef MENU_POWER_DIALOG_H
#define MENU_POWER_DIALOG_H

#include <gtkmm.h>
#include "kiran-opacity-menu.h"

class MenuPowerDialog : public Gtk::Window
{
public:
    MenuPowerDialog();
    ~MenuPowerDialog() override;
    void do_shutdown();
    void do_reboot();
    void do_logout();
    void do_lock();
    void do_hibernate();
    void do_suspend();
    void do_switch_user();
    void do_lock_screen();

    sigc::signal<void> signal_dialog_hide();

    void add_power_button(Gtk::Grid *dialog_grid);
    void add_button(const char *button_icon_name,
                    const char *button_name,
                    Gtk::Grid *dialog_grid,
                    int space,
                    void (MenuPowerDialog::*power_func)());
protected:
    sigc::signal<void> signal_dialog_hide_;
};

#endif  // MENU_POWER_DIALOG_H
