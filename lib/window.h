/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:26:46
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 09:51:14
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window.h
 */

#pragma once

#include <gdkmm.h>
#include <libwnck/libwnck.h>

namespace Kiran
{
class App;
class Window;
class Workspace;

using WindowVec = std::vector<std::shared_ptr<Window>>;

class Window : public std::enable_shared_from_this<Window>
{
   public:
    Window(WnckWindow* window);
    virtual ~Window();

    std::string get_name();

    std::string get_icon_name();

    GdkPixbuf* get_icon();

    GdkPixbuf* get_mini_icon();

    std::shared_ptr<App> get_app();

    std::shared_ptr<Window> get_transient();

    uint64_t get_xid();

    WindowVec get_group_windows();

    std::string get_class_group_name();

    std::string get_class_instance_name();

    int32_t get_pid();

    WnckWindowType get_window_type();

    void activate();

    void minimize();

    void maximize();

    void unmaximize();

    void make_above();

    bool is_active();

    void move_to_workspace(std::shared_ptr<Workspace> workspace);

    void close();

   private:
    WnckWindow* wnck_window_;

    friend class App;
};
}  // namespace Kiran