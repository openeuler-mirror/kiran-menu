/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 12:04:38
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

WindowManager* WindowManager::instance_ = nullptr;
void WindowManager::global_init()
{
    instance_ = new WindowManager();
    instance_->init();
}

void monitor_window_opened(WnckScreen* screen,
                           WnckWindow* window,
                           gpointer user_data)
{
    WindowManager* self = (WindowManager*)user_data;
    self->add_window(window);
}

static void monitor_window_closed(WnckScreen* screen,
                                  WnckWindow* window,
                                  gpointer user_data)
{
    WindowManager* self = (WindowManager*)user_data;
    self->remove_window(window);
}

void WindowManager::init()
{
    load_windows();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    g_signal_connect(screen, "window-opened", G_CALLBACK(monitor_window_opened), this);
    g_signal_connect(screen, "window-closed", G_CALLBACK(monitor_window_closed), this);
}

std::shared_ptr<Window> WindowManager::get_active_window()
{
    auto screen = wnck_screen_get_default();
    g_return_val_if_fail(screen != NULL, nullptr);

    auto wnck_window = wnck_screen_get_active_window(screen);

    return lookup_window(wnck_window);
}

std::shared_ptr<Window> WindowManager::lookup_window(WnckWindow* wnck_window)
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
        auto window = std::make_shared<Window>(wnck_window);
        this->windows_.emplace(xid, window);
        return window;
    }
}

void WindowManager::add_window(WnckWindow* wnck_window)
{
    RETURN_IF_TRUE(wnck_window == NULL);

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = this->windows_.find(xid);
    if (iter != this->windows_.end())
    {
        g_debug("the window already exists. name: %s xid: %" PRIu64 "\n",
                iter->second->get_name().c_str(),
                iter->second->get_xid());
    }
    else
    {
        auto window = std::make_shared<Window>(wnck_window);
        this->windows_.emplace(xid, window);
        this->add_window_.emit(window);
    }
}

void WindowManager::remove_window(WnckWindow* wnck_window)
{
    RETURN_IF_TRUE(wnck_window == NULL);

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);

    auto iter = this->windows_.find(xid);
    if (iter != this->windows_.end())
    {
        auto window = iter->second;
        this->windows_.erase(iter);
        this->remove_window_.emit(window);
    }
    else
    {
        g_warning("the window not exists. name: %s xid: %" PRIu64 "\n",
                  wnck_window_get_name(wnck_window),
                  wnck_window_get_xid(wnck_window));
    }
}

void WindowManager::load_windows()
{
    this->windows_.clear();

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    auto wnck_windows = wnck_screen_get_windows(screen);

    for (GList* l = wnck_windows; l != NULL; l = l->next)
    {
        auto wnck_window = (WnckWindow*)(l->data);
        auto xid = (uint64_t)wnck_window_get_xid(wnck_window);
        auto window = std::make_shared<Window>(wnck_window);
        auto iter = this->windows_.emplace(xid, window);
        if (!iter.second)
        {
            iter.first->second = window;
        }
    }
}

}  // namespace Kiran
