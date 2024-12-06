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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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
