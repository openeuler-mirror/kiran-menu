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
