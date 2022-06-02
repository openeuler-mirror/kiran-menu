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

#include "menu-app-launcher-button.h"

MenuAppLauncherButton::MenuAppLauncherButton(const std::string &icon_name, const char *tooltip, const char *cmdline)
{
    auto context = get_style_context();
    set_tooltip_text(tooltip);

    icon.set_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
    add(icon);
    set_hexpand(true);
    set_halign(Gtk::ALIGN_FILL);

    app = Gio::AppInfo::create_from_commandline(cmdline, std::string(),
                                                Gio::APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION);

    context->add_class("menu-app-launcher");
}

void MenuAppLauncherButton::on_clicked()
{
    std::vector<Glib::RefPtr<Gio::File>> files;

    files.clear();
    app->launch(files);

    signal_app_launched().emit();
}

sigc::signal<void> MenuAppLauncherButton::signal_app_launched()
{
    return m_signal_app_launched;
}
