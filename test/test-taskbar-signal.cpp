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
