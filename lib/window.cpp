/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:26:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-09 10:25:44
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window.cpp
 */

#include "lib/window.h"

#include "lib/app-manager.h"
#include "lib/app.h"
#include "lib/helper.h"
#include "lib/workspace-manager.h"

namespace Kiran
{
std::shared_ptr<Window> Window::create(WnckWindow* wnck_window)
{
    std::shared_ptr<Window> window(new Window(wnck_window));
    window->flush_workspace();
    return window;
}

Window::Window(WnckWindow* wnck_window) : wnck_window_(wnck_window),
                                          last_workspace_number_(-1),
                                          last_is_pinned_(false),
                                          pixmap_(0)
{
    g_signal_connect(this->wnck_window_, "workspace-changed", G_CALLBACK(Window::workspace_changed), NULL);
}

Window::~Window()
{
}

std::string Window::get_name()
{
    RET_WRAP_NULL(wnck_window_get_name(this->wnck_window_));
}

std::string Window::get_icon_name()
{
    RET_WRAP_NULL(wnck_window_get_icon_name(this->wnck_window_));
}

GdkPixbuf* Window::get_icon()
{
    return wnck_window_get_icon(this->wnck_window_);
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
    RET_WRAP_NULL(wnck_window_get_class_group_name(this->wnck_window_));
}

std::string Window::get_class_instance_name()
{
    RET_WRAP_NULL(wnck_window_get_class_instance_name(this->wnck_window_));
}

int32_t Window::get_pid()
{
    return wnck_window_get_pid(this->wnck_window_);
}

WnckWindowType Window::get_window_type()
{
    return wnck_window_get_window_type(this->wnck_window_);
}

bool Window::is_pinned()
{
    return wnck_window_is_pinned(this->wnck_window_);
}

bool Window::is_above()
{
    return wnck_window_is_above(this->wnck_window_);
}

void Window::activate(uint32_t timestamp)
{
    wnck_window_activate(this->wnck_window_, timestamp);
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

uint64_t Window::get_window_group()
{
    return wnck_window_get_group_leader(this->wnck_window_);
}
void Window::close()
{
    uint64_t now = Glib::DateTime::create_now_local().to_unix();
    wnck_window_close(this->wnck_window_, now);
}

std::tuple<int, int, int, int> Window::get_geometry()
{
    int x, y, w, h;
    wnck_window_get_geometry(this->wnck_window_, &x, &y, &w, &h);
    return std::make_tuple(x, y, w, h);
}

std::shared_ptr<Workspace> Window::get_workspace()
{
    auto wnck_workspace = wnck_window_get_workspace(this->wnck_window_);

    return WorkspaceManager::get_instance()->lookup_workspace(wnck_workspace);
}

void Window::flush_workspace()
{
    auto workspace = get_workspace();

    if (workspace)
    {
        // add window
        workspace->add_window(this->shared_from_this());

        // remove window
        int32_t current_number = workspace->get_number();
        if (this->last_workspace_number_ != current_number)
        {
            auto last_workspace = WorkspaceManager::get_instance()->get_workspace(this->last_workspace_number_);
            if (last_workspace)
            {
                last_workspace->remove_window(this->shared_from_this());
            }
            else if (this->last_is_pinned_)
            {
                auto workspaces = WorkspaceManager::get_instance()->get_workspaces();
                for (auto i = 0; i < workspaces.size(); ++i)
                {
                    workspaces[i]->remove_window(this->shared_from_this());
                }
            }
        }
        // update status variable
        this->last_workspace_number_ = current_number;
        this->last_is_pinned_ = false;
    }
    else
    {
        bool current_is_pinned = this->is_pinned();
        // add window
        if (current_is_pinned)
        {
            auto workspaces = WorkspaceManager::get_instance()->get_workspaces();
            for (auto i = 0; i < workspaces.size(); ++i)
            {
                workspaces[i]->add_window(this->shared_from_this());
            }
        }

        // remove window
        if (!current_is_pinned && this->last_workspace_number_ >= 0)
        {
            auto last_workspace = WorkspaceManager::get_instance()->get_workspace(this->last_workspace_number_);
            if (last_workspace)
            {
                last_workspace->remove_window(this->shared_from_this());
            }
        }

        this->last_workspace_number_ = -1;
        this->last_is_pinned_ = current_is_pinned;
    }
}

void Window::workspace_changed(WnckWindow* wnck_window, gpointer user_data)
{
    g_return_if_fail(wnck_window != NULL);

    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    if (window)
    {
        window->flush_workspace();
    }
}

}  // namespace Kiran