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

#pragma once
#include <gdkmm.h>
#include <libwnck/libwnck.h>

namespace Kiran
{
class ScreenManager
{
public:
    virtual ~ScreenManager();

    static ScreenManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

    // 开始更新window/app/workspace相关数据，这里会触发当前打开的所有window/app的opened信号。
    // WindowManager/AppManager通过捕获opened/closed信号来建立对wnck_window/wnck_appplication的创建和销毁。
    void force_update();

    /*设置是否显示桌面(隐藏所有窗口)*/
    void set_show_desktop(bool show);
    bool get_show_desktop();

    // 调用force_update函数触发的信号
    sigc::signal<void> &signal_force_update() { return this->force_update_; }
    sigc::signal<void, bool> signal_show_desktop_changed() { return this->show_desktop_changed; }

private:
    ScreenManager();

private:
    static ScreenManager *instance_;

    WnckScreen *screen_;

    sigc::signal<void> force_update_;
    sigc::signal<void, bool> show_desktop_changed;
};
}  // namespace Kiran
