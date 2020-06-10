/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 10:02:51
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

    std::shared_ptr<Window> get_active_window();

    std::shared_ptr<Window> lookup_window(WnckWindow *wnck_window);

    void add_window(WnckWindow *wnck_window);

    void remove_window(WnckWindow *wnck_window);

    //signal accessor:
    sigc::signal<void, std::shared_ptr<Window>> &signal_add_window() { return this->add_window_; }
    sigc::signal<void, std::shared_ptr<Window>> &signal_remove_window() { return this->remove_window_; }

   private:
    WindowManager();
    void load_windows();

   protected:
    sigc::signal<void, std::shared_ptr<Window>> add_window_;
    sigc::signal<void, std::shared_ptr<Window>> remove_window_;

   private:
    static WindowManager *instance_;

    std::map<uint64_t, std::shared_ptr<Window>> windows_;
};
}  // namespace Kiran
