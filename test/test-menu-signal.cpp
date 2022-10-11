/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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

#include <gtkmm.h>

#include "lib/app.h"
#include "lib/core_worker.h"

void sig_hander(int signo)
{
}

static void installed_changed()
{
    g_print("recv installed-changed signal.\n");
}

static void app_installed(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv app %s installed signal.\n", app->get_desktop_id().c_str());
    }
}

static void app_uninstalled(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv app %s uninstalled signal.\n", app->get_desktop_id().c_str());
    }
}

static void new_app_changed()
{
    g_print("recv new app changed signal.\n");
}

static void favorite_app_added(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv favorite app %s added signal.\n", app->get_desktop_id().c_str());
    }
}

static void favorite_app_deleted(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv favorite app %s deleted signal.\n", app->get_desktop_id().c_str());
    }
}

static void frequent_usage_app_changed()
{
    g_print("recv frequent-usage-app-changed signal.\n");
}

int main(int argc, char **argv)
{
    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    auto menu = Kiran::MenuSkeleton::get_instance();

    menu->signal_app_changed().connect(&installed_changed);
    menu->signal_app_installed().connect(&app_installed);
    menu->signal_app_uninstalled().connect(&app_uninstalled);
    menu->signal_new_app_changed().connect(&new_app_changed);
    menu->signal_favorite_app_added().connect(&favorite_app_added);
    menu->signal_favorite_app_deleted().connect(&favorite_app_deleted);
    menu->signal_frequent_usage_app_changed().connect(&frequent_usage_app_changed);

    kit.run();

    return 0;
}
