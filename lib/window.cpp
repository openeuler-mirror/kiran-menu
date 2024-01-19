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

#include "lib/window.h"

#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <cairo/cairo-xlib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <xcb/res.h>

#include "lib/app-manager.h"
#include "lib/app.h"
#include "lib/base.h"
#include "lib/workspace-manager.h"

namespace Kiran
{
Window::Window(WnckWindow *wnck_window) : wnck_window_(wnck_window),
                                          gdk_window_(NULL),
                                          last_workspace_number_(-1),
                                          last_is_pinned_(false),
                                          pixmap_(None),
                                          last_geometry_(0, 0, 0, 0),
                                          name_changed_handler_(0),
                                          workspace_changed_handler_(0),
                                          geometry_changed_handler_(0),
                                          state_changed_handler(0)
{
    xid_ = wnck_window_get_xid(wnck_window_);
    auto workspace = this->get_workspace();
    if (workspace)
    {
        this->last_workspace_number_ = workspace->get_number();
    }
    this->last_is_pinned_ = this->is_pinned();

    this->name_changed_handler_ = g_signal_connect(this->wnck_window_, "name-changed", G_CALLBACK(Window::name_changed), NULL);
    this->workspace_changed_handler_ = g_signal_connect(this->wnck_window_, "workspace-changed", G_CALLBACK(Window::workspace_changed), NULL);
    this->geometry_changed_handler_ = g_signal_connect(this->wnck_window_, "geometry-changed", G_CALLBACK(Window::geometry_changed), NULL);
    this->state_changed_handler = g_signal_connect(this->wnck_window_, "state-changed", G_CALLBACK(Window::state_changed), NULL);
    this->update_window_pixmap();
}

Window::Window(gulong xid) : wnck_window_(nullptr),
                             gdk_window_(NULL),
                             xid_(xid),
                             last_workspace_number_(-1),
                             last_is_pinned_(false),
                             pixmap_(None),
                             last_geometry_(0, 0, 0, 0),
                             name_changed_handler_(0),
                             workspace_changed_handler_(0),
                             geometry_changed_handler_(0),
                             state_changed_handler(0)
{
    wnck_window_ = wnck_window_get(xid);
    gdk_window_ = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), xid);
    if (gdk_window_ != nullptr)
        g_object_ref(gdk_window_);

    auto workspace = this->get_workspace();
    if (workspace)
    {
        this->last_workspace_number_ = workspace->get_number();
    }
    this->last_is_pinned_ = this->is_pinned();

    if (wnck_window_ != nullptr)
    {
        this->name_changed_handler_ = g_signal_connect(this->wnck_window_, "name-changed", G_CALLBACK(Window::name_changed), NULL);
        this->workspace_changed_handler_ = g_signal_connect(this->wnck_window_, "workspace-changed", G_CALLBACK(Window::workspace_changed), NULL);
        this->geometry_changed_handler_ = g_signal_connect(this->wnck_window_, "geometry-changed", G_CALLBACK(Window::geometry_changed), NULL);
        this->state_changed_handler = g_signal_connect(this->wnck_window_, "state-changed", G_CALLBACK(Window::state_changed), NULL);
    }
    else
    {
        KLOG_WARNING("No WnckWindow found for Window with ID 0x%x", xid);
    }
    this->update_window_pixmap();
}

Window::~Window()
{
    auto display = gdk_x11_get_default_xdisplay();
    g_return_if_fail(display);

    if (this->pixmap_)
    {
        XFreePixmap(display, this->pixmap_);
        this->pixmap_ = None;
    }

    if (this->load_pixmap_)
    {
        this->load_pixmap_.disconnect();
    }

    if (this->wnck_window_)
    {
        if (this->name_changed_handler_)
        {
            g_signal_handler_disconnect(this->wnck_window_, this->name_changed_handler_);
        }

        if (this->workspace_changed_handler_)
        {
            g_signal_handler_disconnect(this->wnck_window_, this->workspace_changed_handler_);
        }

        if (this->geometry_changed_handler_)
        {
            g_signal_handler_disconnect(this->wnck_window_, this->geometry_changed_handler_);
        }

        if (this->state_changed_handler)
            g_signal_handler_disconnect(this->wnck_window_, this->state_changed_handler);
    }

    if (gdk_window_ != nullptr)
        g_object_unref(gdk_window_);
}

std::string Window::get_name()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, "");
    RET_WRAP_NULL(wnck_window_get_name(this->wnck_window_));
}

std::string Window::get_icon_name()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, "");
    RET_WRAP_NULL(wnck_window_get_icon_name(this->wnck_window_));
}

GdkPixbuf *Window::get_icon()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, nullptr);
    return wnck_window_get_icon(this->wnck_window_);
}

cairo_surface_t *Window::get_thumbnail(int &thumbnail_width, int &thumbnail_height)
{
    Display *xdisplay;
    GdkDisplay *display = gdk_display_get_default();
    XWindowAttributes attrs;
    cairo_surface_t *surface = nullptr;
    Pixmap pixmap = get_pixmap();

    xdisplay = GDK_DISPLAY_XDISPLAY(display);
    if (pixmap != None)
    {
        gdk_x11_display_error_trap_push(display);
        XGetWindowAttributes(xdisplay, get_xid(), &attrs);
        surface = cairo_xlib_surface_create(xdisplay,
                                            pixmap,
                                            attrs.visual,
                                            attrs.width,
                                            attrs.height);
        if (gdk_x11_display_error_trap_pop(display))
            surface = nullptr;
        thumbnail_width = attrs.width;
        thumbnail_height = attrs.height;
    }

    return surface;
}

std::shared_ptr<App> Window::get_app()
{
    return AppManager::get_instance()->lookup_app_with_window(this->shared_from_this());
}

uint64_t Window::get_xid()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr || this->gdk_window_ != nullptr, 0);
    return xid_;
}

WindowVec Window::get_group_windows()
{
    WindowVec windows;

    g_return_val_if_fail(this->wnck_window_ != nullptr, WindowVec());
    auto group = wnck_window_get_class_group(this->wnck_window_);
    if (!group)
    {
        return windows;
    }

    auto wnck_windows = wnck_class_group_get_windows(group);

    for (auto l = wnck_windows; l != NULL; l = l->next)
    {
        auto wnck_window = (WnckWindow *)(l->data);
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
    g_return_val_if_fail(this->wnck_window_ != nullptr, std::shared_ptr<Window>());
    auto wnck_window = wnck_window_get_transient(this->wnck_window_);
    return WindowManager::get_instance()->lookup_window(wnck_window);
}

std::string Window::get_class_group_name()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, "");
    RET_WRAP_NULL(wnck_window_get_class_group_name(this->wnck_window_));
}

void Window::keyboard_move()
{
    g_return_if_fail(this->wnck_window_ != nullptr);
    wnck_window_keyboard_move(this->wnck_window_);
}

bool Window::needs_attention()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_needs_attention(this->wnck_window_);
}

std::string Window::get_class_instance_name()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, "");
    RET_WRAP_NULL(wnck_window_get_class_instance_name(this->wnck_window_));
}

int32_t Window::get_pid()
{
    /*没有使用wnck_window_get_pid获取窗口pid，因为如果窗口对应的是flatpak应用，
    函数返回的是沙盒内的进程ID，这里参考mutter的代码，直接向xserver发送请求，
    对于flatpak应用，这里获取的是沙盒外的进程ID*/

    auto xdisplay = gdk_x11_get_default_xdisplay();
    xcb_connection_t *xcb = XGetXCBConnection(xdisplay);
    xcb_res_client_id_spec_t spec = {0};
    xcb_res_query_client_ids_cookie_t cookie;
    xcb_res_query_client_ids_reply_t *reply = NULL;

    spec.client = this->get_xid();
    spec.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;

    cookie = xcb_res_query_client_ids(xcb, 1, &spec);
    reply = xcb_res_query_client_ids_reply(xcb, cookie, NULL);

    if (reply == NULL)
        return 0;

    uint32_t pid = 0, *value;
    xcb_res_client_id_value_iterator_t it;
    for (it = xcb_res_query_client_ids_ids_iterator(reply);
         it.rem;
         xcb_res_client_id_value_next(&it))
    {
        spec = it.data->spec;
        if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID)
        {
            value = xcb_res_client_id_value_value(it.data);
            pid = *value;
            break;
        }
    }

    free(reply);
    return pid;
}

WnckWindowType Window::get_window_type()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, WNCK_WINDOW_NORMAL);
    return wnck_window_get_window_type(this->wnck_window_);
}

bool Window::is_pinned()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_is_pinned(this->wnck_window_);
}

bool Window::is_above()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_is_above(this->wnck_window_);
}

bool Window::is_skip_pager()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_is_skip_pager(this->wnck_window_);
}

bool Window::is_skip_taskbar()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_is_skip_tasklist(this->wnck_window_);
}

void Window::activate(uint32_t timestamp)
{
    KLOG_PROFILE("xid: %" PRIu64 ", timestamp: %d.", this->get_xid(), timestamp);

    g_return_if_fail(this->wnck_window_ != nullptr);

    WnckWindowState state = wnck_window_get_state(wnck_window_);
    auto workspace = get_workspace();
    auto current_workspace = WorkspaceManager::get_instance()->get_active_workspace();

    if (state & WNCK_WINDOW_STATE_MINIMIZED)
    {
        if (workspace && workspace != current_workspace)
            workspace->activate(timestamp);
        wnck_window_activate_transient(this->wnck_window_, timestamp);
    }
    else if (wnck_window_is_active(wnck_window_) ||
             wnck_window_is_most_recently_activated(wnck_window_) ||
             wnck_window_transient_is_most_recently_activated(wnck_window_))
    {
        wnck_window_minimize(wnck_window_);
    }
    else
    {
        if (workspace && workspace != current_workspace)
            workspace->activate(timestamp);
        wnck_window_activate_transient(this->wnck_window_, timestamp);
    }
}

void Window::unminimize(uint32_t timestamp)
{
    KLOG_PROFILE("xid: %" PRIu64 ", timestamp: %d.", this->get_xid(), timestamp);
    if (this->wnck_window_ != nullptr)
        wnck_window_unminimize(this->wnck_window_, timestamp);
    else
        gdk_window_deiconify(gdk_window_);
}

bool Window::is_minimized()
{
    if (this->wnck_window_ != nullptr)
        return wnck_window_is_minimized(this->wnck_window_);
    else
    {
        GdkWindowState state = gdk_window_get_state(gdk_window_);

        return (state & GDK_WINDOW_STATE_ICONIFIED) != 0;
    }
}

void Window::minimize()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        wnck_window_minimize(this->wnck_window_);
    else
        gdk_window_iconify(gdk_window_);
}

bool Window::is_maximized()
{
    if (this->wnck_window_ != nullptr)
        return wnck_window_is_maximized(this->wnck_window_);
    else
    {
        GdkWindowState state = gdk_window_get_state(gdk_window_);

        return (state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    }
}

void Window::maximize()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        wnck_window_maximize(this->wnck_window_);
    else
        gdk_window_maximize(gdk_window_);
}

void Window::unmaximize()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        wnck_window_unmaximize(this->wnck_window_);
    else
        gdk_window_unmaximize(gdk_window_);
}

bool Window::is_shaded()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, false);
    return wnck_window_is_shaded(this->wnck_window_);
}

void Window::pin()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        return wnck_window_pin(this->wnck_window_);
    else
        gdk_window_stick(gdk_window_);
}

void Window::unpin()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        return wnck_window_unpin(this->wnck_window_);
    else
        gdk_window_unstick(gdk_window_);
}

void Window::make_above()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        wnck_window_make_above(this->wnck_window_);
    else
        gdk_window_set_keep_above(gdk_window_, true);
}

void Window::make_unabove()
{
    KLOG_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    if (this->wnck_window_ != nullptr)
        wnck_window_unmake_above(this->wnck_window_);
    else
        gdk_window_set_keep_above(gdk_window_, false);
}

bool Window::is_active()
{
    if (this->wnck_window_ != nullptr)
        return wnck_window_is_active(this->wnck_window_);
    else
    {
        return gdk_screen_get_active_window(gdk_window_get_screen(gdk_window_)) == gdk_window_;
    }
}

void Window::move_to_workspace(std::shared_ptr<Workspace> workspace)
{
    KLOG_PROFILE("xid: %" PRIu64 ", number: %d.",
                 this->get_xid(),
                 workspace ? workspace->get_number() : -1);

    if (this->wnck_window_ != nullptr)
        wnck_window_move_to_workspace(this->wnck_window_, workspace->workspace_);
    else
    {
        gdk_x11_window_move_to_desktop(gdk_window_, workspace->get_number());
    }
}

uint64_t Window::get_window_group()
{
    g_return_val_if_fail(this->wnck_window_ != nullptr, 0);
    return wnck_window_get_group_leader(this->wnck_window_);
}

void Window::close()
{
    KLOG_PROFILE("xid: %" PRIu64 ", name: %s.", this->get_xid(), this->get_name().c_str());

    g_return_if_fail(this->wnck_window_ != nullptr);
    wnck_window_close(this->wnck_window_, gtk_get_current_event_time());
}

WindowGeometry Window::get_geometry()
{
    int x, y, w, h;
    if (this->wnck_window_ != nullptr)
        wnck_window_get_geometry(this->wnck_window_, &x, &y, &w, &h);
    else
        gdk_window_get_geometry(gdk_window_, &x, &y, &w, &h);
    return std::make_tuple(x, y, w, h);
}

WindowGeometry Window::get_client_window_geometry()
{
    int x, y, w, h;
    if (this->wnck_window_ != nullptr)
        wnck_window_get_client_window_geometry(this->wnck_window_, &x, &y, &w, &h);
    else
        gdk_window_get_geometry(gdk_window_, &x, &y, &w, &h);
    return std::make_tuple(x, y, w, h);
}

void Window::set_geometry(WnckWindowGravity gravity,
                          WnckWindowMoveResizeMask geometry_mask,
                          int x,
                          int y,
                          int width,
                          int height)
{
    wnck_window_set_geometry(this->wnck_window_, gravity, geometry_mask, x, y, width, height);
}

std::shared_ptr<Workspace> Window::get_workspace()
{
    if (wnck_window_ != nullptr)
    {
        auto wnck_workspace = wnck_window_get_workspace(this->wnck_window_);
        return WorkspaceManager::get_instance()->lookup_workspace(wnck_workspace);
    }
    else
    {
        /* 从GDK接口获取工作区 */
        g_return_val_if_fail(gdk_window_ != nullptr, std::shared_ptr<Workspace>());
        guint32 workspace_no = gdk_x11_window_get_desktop(gdk_window_);
        return WorkspaceManager::get_instance()->get_workspace(workspace_no);
    }
}

void Window::flush_workspace()
{
    KLOG_PROFILE("");

    auto workspace = get_workspace();

    if (workspace)
    {
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
                for (auto i = 0; i < (int)workspaces.size(); ++i)
                {
                    workspaces[i]->remove_window(this->shared_from_this());
                }
            }
        }

        // add window
        workspace->add_window(this->shared_from_this());

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
            for (auto i = 0; i < (int)workspaces.size(); ++i)
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

void Window::name_changed(WnckWindow *wnck_window, gpointer user_data)
{
    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    g_return_if_fail(window);

    KLOG_DEBUG("the name of the window %" PRIu64 " is changed. new_name: %s.", window->get_xid(), window->get_name().c_str());

    window->name_changed_.emit();
}

void Window::workspace_changed(WnckWindow *wnck_window, gpointer user_data)
{
    g_return_if_fail(wnck_window != NULL);

    auto xid = wnck_window_get_xid(wnck_window);
    auto name = wnck_window_get_name(wnck_window);

    KLOG_DEBUG("the workspace of the window is changed. xid: %" PRIu64 ", name: %s.", xid, name);

    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    if (window)
    {
        auto last_workspace = WorkspaceManager::get_instance()->get_workspace(window->last_workspace_number_);
        auto cur_workspace = window->get_workspace();
        window->flush_workspace();
        window->workspace_changed_.emit(last_workspace, cur_workspace);
    }
    else
    {
        KLOG_WARNING("cannot find the window for wnck window: %" PRIu64 ".", xid);
    }
}

void Window::geometry_changed(WnckWindow *wnck_window,
                              gpointer user_data)
{
    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    g_return_if_fail(window);

    KLOG_DEBUG("the geometry of the window is changed. xid: %" PRIu64 ", window name: %s.",
               window->get_xid(),
               window->get_name().c_str());

    auto display = gdk_x11_get_default_xdisplay();
    auto gdk_screen = gdk_screen_get_default();

    g_return_if_fail(display);
    g_return_if_fail(gdk_screen);

    auto geometry = window->get_geometry();
    if (gdk_screen_is_composited(gdk_screen))
    {
        if (std::get<2>(geometry) != std::get<2>(window->last_geometry_) ||
            std::get<3>(geometry) != std::get<3>(window->last_geometry_))
        {
            if (!window->load_pixmap_)
            {
                auto timeout = Glib::MainContext::get_default()->signal_timeout();
                window->load_pixmap_ = timeout.connect(sigc::mem_fun(window.get(), &Window::update_window_pixmap), 200);
            }
        }
    }
    else
    {
        KLOG_WARNING("the extension composite is unsupported.\n");
    }

    window->last_geometry_ = geometry;
}

void Window::state_changed(WnckWindow *wnck_window, gpointer user_data)
{
    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    g_return_if_fail(window);

    window->signal_state_changed().emit();
}

void Window::process_events(GdkXEvent *xevent, GdkEvent *event)
{
    auto x_event = (XEvent *)xevent;

    if (x_event->type == MapNotify || x_event->type == ConfigureNotify)
    {
        this->update_window_pixmap();
    }
}

bool Window::update_window_pixmap()
{
    auto display = gdk_x11_get_default_xdisplay();
    g_return_val_if_fail(display, false);

    auto gdk_display = gdk_x11_lookup_xdisplay(display);
    g_return_val_if_fail(gdk_display, false);

    if (this->pixmap_)
    {
        XFreePixmap(display, this->pixmap_);
        this->pixmap_ = None;
    }

    gdk_x11_display_error_trap_push(gdk_display);
    this->pixmap_ = XCompositeNameWindowPixmap(display, this->get_xid());

    if (gdk_x11_display_error_trap_pop(gdk_display))
    {
        this->pixmap_ = None;
    }
    return false;
}

void Window::set_on_visible_workspace(bool on)
{
    if (on)
        this->pin();
    else
        this->unpin();
}

bool Window::get_on_visible_workspace()
{
    return is_pinned();
}

bool Window::should_skip_taskbar()
{
    if (is_skip_pager() || is_skip_taskbar())
        return true;
    return get_window_type() != WNCK_WINDOW_NORMAL && get_window_type() != WNCK_WINDOW_DIALOG;
}

void Window::set_icon_geometry(int x, int y, int width, int height)
{
    g_return_if_fail(this->wnck_window_ != nullptr);
    wnck_window_set_icon_geometry(this->wnck_window_,
                                  x,
                                  y,
                                  width,
                                  height);
}
}  // namespace Kiran
