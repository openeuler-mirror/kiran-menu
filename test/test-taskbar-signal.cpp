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

static void fixed_app_added(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv fixed app %s added signal.\n", app->get_desktop_id().c_str());
    }
}

static void fixed_app_deleted(Kiran::AppVec apps)
{
    for (auto app : apps)
    {
        g_print("recv fixed app %s deleted signal.\n", app->get_desktop_id().c_str());
    }
}

int main(int argc, char **argv)
{
    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    auto taskbar = Kiran::TaskBarSkeleton::get_instance();

    taskbar->signal_fixed_app_added().connect(&fixed_app_added);
    taskbar->signal_fixed_app_deleted().connect(&fixed_app_deleted);

    kit.run();

    return 0;
}
