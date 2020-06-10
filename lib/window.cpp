/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:26:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 10:03:46
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window.cpp
 */

#include "lib/window.h"

#include "lib/app-manager.h"
#include "lib/app.h"
#include "lib/workspace.h"

namespace Kiran
{
Window::Window(WnckWindow* window) : wnck_window_(window)
{
}

Window::~Window()
{
}

std::string Window::get_name()
{
    return wnck_window_get_name(this->wnck_window_);
}

std::string Window::get_icon_name()
{
    return wnck_window_get_icon_name(this->wnck_window_);
}

GdkPixbuf* Window::get_icon()
{
    return wnck_window_get_icon(this->wnck_window_);
}

GdkPixbuf* Window::get_mini_icon()
{
    return wnck_window_get_mini_icon(this->wnck_window_);
}

std::shared_ptr<App> Window::get_app()
{
    return AppManager::get_instance()->lookup_app_with_window(this->shared_from_this());
}

uint64_t Window::get_xid()
{
    return (uint64_t)wnck_window_get_xid(this->wnck_window_);
}

WindowVec Window::get_group_windows()
{
    WindowVec windows;
    auto group = wnck_window_get_class_group(this->wnck_window_);
    if (!group)
    {
        return windows;
    }

    auto wnck_windows = wnck_class_group_get_windows(group);

    for (auto l = wnck_windows; l != NULL; l = l->next)
    {
        auto wnck_window = (WnckWindow*)(l->data);
        auto window = WindowManager::get_instance()->lookup_window(wnck_window);
        if (window)
        {
            windows.push_back(window);
        }
    }
    return windows;
}

std::shared_ptr<Window> Window::get_transient()
{
    auto wnck_window = wnck_window_get_transient(this->wnck_window_);
    return WindowManager::get_instance()->lookup_window(wnck_window);
}

std::string Window::get_class_group_name()
{
    return wnck_window_get_class_group_name(this->wnck_window_);
}

std::string Window::get_class_instance_name()
{
    return wnck_window_get_class_instance_name(this->wnck_window_);
}

int32_t Window::get_pid()
{
    return wnck_window_get_pid(this->wnck_window_);
}

WnckWindowType Window::get_window_type()
{
    return wnck_window_get_window_type(this->wnck_window_);
}

void Window::activate()
{
    uint64_t now = Glib::DateTime::create_now_local().to_unix();
    wnck_window_activate(this->wnck_window_, (uint32_t)now);
}

void Window::minimize()
{
    wnck_window_minimize(this->wnck_window_);
}

void Window::maximize()
{
    wnck_window_maximize(this->wnck_window_);
}

void Window::unmaximize()
{
    wnck_window_unmaximize(this->wnck_window_);
}

void Window::make_above()
{
    wnck_window_make_above(this->wnck_window_);
}

bool Window::is_active()
{
    return wnck_window_is_active(this->wnck_window_);
}

void Window::move_to_workspace(std::shared_ptr<Workspace> workspace)
{
    wnck_window_move_to_workspace(this->wnck_window_, workspace->workspace_);
}

void Window::close()
{
    uint64_t now = Glib::DateTime::create_now_local().to_unix();
    wnck_window_close(this->wnck_window_, now);
}

}  // namespace Kiran