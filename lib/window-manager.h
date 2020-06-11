/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 14:22:13
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window-manager.h
 */

#pragma once

#include "lib/window.h"

namespace Kiran
{
class WindowManager
{
   public:
    virtual ~WindowManager();

    static WindowManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

    // 通过xid获取窗口
    std::shared_ptr<Window> get_window(uint64_t xid);

    // 获取当前激活窗口
    std::shared_ptr<Window> get_active_window();

    // 获取所有窗口
    WindowVec get_windows();

    std::shared_ptr<Window> lookup_window(WnckWindow *wnck_window);

    // 打开窗口信号
    sigc::signal<void, std::shared_ptr<Window>> &signal_window_opened() { return this->window_opened_; }
    // 关闭窗口信号
    sigc::signal<void, std::shared_ptr<Window>> &signal_window_closed() { return this->window_closed_; }

   private:
    WindowManager();
    void load_windows();

    static void window_opened(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data);

    static void window_closed(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data);

   protected:
    sigc::signal<void, std::shared_ptr<Window>> window_opened_;
    sigc::signal<void, std::shared_ptr<Window>> window_closed_;

   private:
    static WindowManager *instance_;

    std::map<uint64_t, std::shared_ptr<Window>> windows_;
};
}  // namespace Kiran
