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
