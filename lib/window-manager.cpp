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

#include "lib/window-manager.h"

#include <X11/extensions/Xcomposite.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <cinttypes>

#include "lib/base.h"

namespace Kiran
{
WindowManager::WindowManager(ScreenManager *screen_manager) : screen_manager_(screen_manager),
                                                              window_opened_handler_(0),
                                                              window_closed_handler_(0),
                                                              active_window_changed_handler_(0)
{
}

WindowManager::~WindowManager()
{
    auto gdk_display = gdk_display_get_default();
    if (gdk_display)
    {
        gdk_window_remove_filter(NULL, &WindowManager::event_filter, this);
    }

    auto screen = wnck_screen_get_default();
    if (screen)
    {
        if (this->window_opened_handler_)
        {
            g_signal_handler_disconnect(screen, this->window_opened_handler_);
        }
        if (this->window_closed_handler_)
        {
            g_signal_handler_disconnect(screen, this->window_closed_handler_);
        }
        if (this->active_window_changed_handler_)
        {
            g_signal_handler_disconnect(screen, this->active_window_changed_handler_);
        }
    }
}

WindowManager *WindowManager::instance_ = nullptr;
void WindowManager::global_init(ScreenManager *screen_manager)
{
    instance_ = new WindowManager(screen_manager);
    instance_->init();
}

void WindowManager::init()
{
    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    this->window_opened_handler_ = g_signal_connect(screen, "window-opened", G_CALLBACK(WindowManager::window_opened), this);
    this->window_closed_handler_ = g_signal_connect(screen, "window-closed", G_CALLBACK(WindowManager::window_closed), this);
    this->active_window_changed_handler_ = g_signal_connect(screen, "active-window-changed", G_CALLBACK(WindowManager::active_window_changed), this);

    auto gdk_display = gdk_display_get_default();
    if (gdk_display)
    {
        gdk_window_add_filter(NULL, &WindowManager::event_filter, this);
    }
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

std::shared_ptr<Window> WindowManager::create_temp_window(WnckWindow *wnck_window)
{
    return std::make_shared<Window>(wnck_window);
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
        KLOG_DEBUG("not found the wnck_window: %p, xid: %" PRIu64 ", the window maybe have just closed.\n", wnck_window, xid);
        return nullptr;
    }
}

GdkFilterReturn WindowManager::event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    auto window_manager = (WindowManager *)data;
    g_return_val_if_fail(window_manager == WindowManager::get_instance(), GDK_FILTER_REMOVE);
    RETURN_VAL_IF_FALSE(xevent != NULL, GDK_FILTER_CONTINUE);

    auto xid = ((XAnyEvent *)xevent)->window;
    auto window = window_manager->get_window(xid);
    if (window)
    {
        window->process_events(xevent, event);
    }
    return GDK_FILTER_CONTINUE;
}

void WindowManager::window_opened(WnckScreen *screen, WnckWindow *wnck_window, gpointer user_data)
{
    auto window_manager = (WindowManager *)user_data;

    g_return_if_fail(wnck_window != NULL);
    g_return_if_fail(window_manager == WindowManager::get_instance());

    auto xid = (uint64_t)wnck_window_get_xid(wnck_window);
    auto name = wnck_window_get_name(wnck_window);

    KLOG_DEBUG("window is opened. xid: %" PRIu64 ", name: %s.", xid, name);

    auto iter = window_manager->windows_.find(xid);
    if (iter != window_manager->windows_.end())
    {
        KLOG_WARNING("the window already exists. name: %s xid: %" PRIu64 "\n",
                     iter->second->get_name().c_str(),
                     iter->second->get_xid());
    }
    else
    {
        auto window = std::make_shared<Window>(wnck_window);
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
    auto name = wnck_window_get_name(wnck_window);

    KLOG_DEBUG("window is closed. xid: %" PRIu64 ", name: %s.", xid, name);

    auto iter = window_manager->windows_.find(xid);
    if (iter != window_manager->windows_.end())
    {
        auto window = iter->second;
        window_manager->windows_.erase(iter);
        window_manager->window_closed_.emit(window);
    }
    else
    {
        KLOG_WARNING("the window not exists. name: %s xid: %" PRIu64 "\n", name, xid);
    }
}

void WindowManager::active_window_changed(WnckScreen *screen, WnckWindow *prev_wnck_window, gpointer user_data)
{
    auto window_manager = (WindowManager *)user_data;

    g_return_if_fail(window_manager == WindowManager::get_instance());

    auto prev_window = window_manager->lookup_window(prev_wnck_window);
    auto cur_window = window_manager->get_active_window();

    KLOG_DEBUG("prev_xid: %" PRIu64 ", prev_name: %s, cur_xid: %" PRIu64 ", cur_name: %s.",
               prev_window ? prev_window->get_xid() : 0,
               prev_window ? prev_window->get_name().c_str() : "null",
               cur_window ? cur_window->get_xid() : 0,
               cur_window ? cur_window->get_name().c_str() : "null");

    window_manager->active_window_changed_.emit(prev_window, cur_window);
}
}  // namespace Kiran
