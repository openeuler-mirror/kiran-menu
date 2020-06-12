/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-12 11:55:55
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window-manager.cpp
 */

#include "lib/window-manager.h"

#include <cinttypes>

#include "lib/helper.h"

namespace Kiran
{
WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
}

WindowManager *WindowManager::instance_ = nullptr;
void WindowManager::global_init()
{
    instance_ = new WindowManager();
    instance_->init();
}

void WindowManager::init()
{
    // load_windows();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    g_signal_connect(screen, "window-opened", G_CALLBACK(WindowManager::window_opened), this);
    g_signal_connect(screen, "window-closed", G_CALLBACK(WindowManager::window_closed), this);
    g_signal_connect(screen, "active-window-changed", G_CALLBACK(WindowManager::active_window_changed), this);
}

std::shared_ptr<Window> WindowManager::get_window(uint64_t xid)
{
    auto iter = this->windows_.find(xid);
    if (iter == this->windows_.end())
    {
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<Window> WindowManager::get_active_window()
{
    auto screen = wnck_screen_get_default();
    g_return_val_if_fail(screen != NULL, nullptr);

    auto wnck_window = wnck_screen_get_active_window(screen);

    return lookup_window(wnck_window);
}

WindowVec WindowManager::get_windows()
{
    WindowVec windows;
    for (auto iter = this->windows_.begin(); iter != this->windows_.end(); ++iter)
    {
        windows.push_back(iter->second);
    }
    return windows;
}

std::shared_ptr<Window> WindowManager::lookup_window(WnckWindow *wnck_window)
{
    if (!wnck_window)
    {
        return nullptr;
    }

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = this->windows_.find(xid);
    if (iter != this->windows_.end())
    {
        return iter->second;
    }
    else
    {
        g_debug("not found the wnck_window: %p, xid: %" PRIu64 ", the window maybe have just closed.\n", wnck_window, xid);
        return nullptr;
    }
}

std::shared_ptr<Window> WindowManager::lookup_and_create_window(WnckWindow *wnck_window)
{
    if (!wnck_window)
    {
        return nullptr;
    }

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = this->windows_.find(xid);
    if (iter != this->windows_.end())
    {
        return iter->second;
    }
    else
    {
        auto window = Window::create(wnck_window);
        this->windows_.emplace(xid, window);
        return window;
    }
    return nullptr;
}

void WindowManager::load_windows()
{
    this->windows_.clear();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    auto wnck_windows = wnck_screen_get_windows(screen);

    for (GList *l = wnck_windows; l != NULL; l = l->next)
    {
        auto wnck_window = (WnckWindow *)(l->data);
        auto xid = (uint64_t)wnck_window_get_xid(wnck_window);
        auto window = Window::create(wnck_window);
        auto iter = this->windows_.emplace(xid, window);
        if (!iter.second)
        {
            iter.first->second = window;
        }
    }
}

void WindowManager::window_opened(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data)
{
    auto window_manager = (WindowManager *)user_data;

    g_return_if_fail(wnck_window != NULL);
    g_return_if_fail(window_manager == WindowManager::get_instance());

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = window_manager->windows_.find(xid);
    if (iter != window_manager->windows_.end())
    {
        g_debug("the window already exists. name: %s xid: %" PRIu64 "\n",
                iter->second->get_name().c_str(),
                iter->second->get_xid());
    }
    else
    {
        auto window = Window::create(wnck_window);
        window_manager->windows_.emplace(xid, window);
        window_manager->window_opened_.emit(window);
    }
}

void WindowManager::window_closed(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data)
{
    auto window_manager = (WindowManager *)user_data;

    g_return_if_fail(wnck_window != NULL);
    g_return_if_fail(window_manager == WindowManager::get_instance());

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = window_manager->windows_.find(xid);
    if (iter != window_manager->windows_.end())
    {
        auto window = iter->second;
        window_manager->windows_.erase(iter);
        window_manager->window_closed_.emit(window);
    }
    else
    {
        g_warning("the window not exists. name: %s xid: %" PRIu64 "\n",
                  wnck_window_get_name(wnck_window),
                  wnck_window_get_xid(wnck_window));
    }
}

void WindowManager::active_window_changed(WnckScreen *screen, WnckWindow *prev_wnck_window, gpointer user_data)
{
    auto window_manager = (WindowManager *)user_data;

    g_return_if_fail(window_manager == WindowManager::get_instance());

    auto prev_window = window_manager->lookup_window(prev_wnck_window);
    auto cur_window = window_manager->get_active_window();

    window_manager->active_window_changed_.emit(prev_window, cur_window);
}
}  // namespace Kiran
