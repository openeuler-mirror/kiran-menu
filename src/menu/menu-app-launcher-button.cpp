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

#include "menu-app-launcher-button.h"

MenuAppLauncherButton::MenuAppLauncherButton(const char *icon_file, const char *tooltip, const char *cmdline)
{
    auto context = get_style_context();
    set_tooltip_text(tooltip);

    icon.set_from_resource(icon_file);
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
