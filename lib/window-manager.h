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

#include "lib/screen-manager.h"
#include "lib/window.h"

namespace Kiran
{
class WindowManager
{
public:
    virtual ~WindowManager();

    static WindowManager *get_instance() { return instance_; };

    static void global_init(ScreenManager *screen_manager);

    static void global_deinit() { delete instance_; };

    void init();

    // 通过xid获取窗口
    std::shared_ptr<Window> get_window(uint64_t xid);

    // 获取当前激活窗口
    std::shared_ptr<Window> get_active_window();

    // 获取所有窗口
    WindowVec get_windows();

    // 创建一个临时窗口，WindowManager不进行维护
    std::shared_ptr<Window> create_temp_window(WnckWindow *wnck_window);

    // 通过wnck_window查找对应的Window对象
    std::shared_ptr<Window> lookup_window(WnckWindow *wnck_window);

    // 打开窗口信号
    sigc::signal<void, std::shared_ptr<Window>> &signal_window_opened() { return this->window_opened_; }
    // 关闭窗口信号
    sigc::signal<void, std::shared_ptr<Window>> &signal_window_closed() { return this->window_closed_; }
    // 激活窗口发生变化，参数分别为：返回值，上一次激活窗口，当前激活窗口
    sigc::signal<void, std::shared_ptr<Window>, std::shared_ptr<Window>> &signal_active_window_changed() { return this->active_window_changed_; }

private:
    WindowManager(ScreenManager *screen_manager);

    static GdkFilterReturn event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);

    // 处理窗口打开信号
    static void window_opened(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data);
    // 处理窗口关闭信号
    static void window_closed(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data);
    // 处理激活窗口变化信号
    static void active_window_changed(WnckScreen *screen, WnckWindow *prev_wnck_window, gpointer user_data);

protected:
    sigc::signal<void, std::shared_ptr<Window>> window_opened_;
    sigc::signal<void, std::shared_ptr<Window>> window_closed_;
    sigc::signal<void, std::shared_ptr<Window>, std::shared_ptr<Window>> active_window_changed_;

private:
    static WindowManager *instance_;

    ScreenManager *screen_manager_;

    uint64_t window_opened_handler_;
    uint64_t window_closed_handler_;
    uint64_t active_window_changed_handler_;

    std::map<uint64_t, std::shared_ptr<Window>> windows_;
};
}  // namespace Kiran
