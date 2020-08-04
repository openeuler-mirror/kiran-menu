/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-04 08:47:01
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window-manager.h
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

    // 查找和创建Window对象，请确保wnck_window的window-closed信号发出前使用该函数。
    // window-closed信号发送后wnck_window并不会马上销毁，可能还会调用active-window-changed信号，最后才会释放改wnck_window对象
    std::shared_ptr<Window> lookup_and_create_window(WnckWindow *wnck_window);

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

    std::map<uint64_t, std::shared_ptr<Window>> windows_;
};
}  // namespace Kiran
