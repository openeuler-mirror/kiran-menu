/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:26:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-09 10:39:17
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/window.cpp
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
#include "lib/log.h"
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
}

std::string Window::get_name()
{
    RET_WRAP_NULL(wnck_window_get_name(this->wnck_window_));
}

std::string Window::get_icon_name()
{
    RET_WRAP_NULL(wnck_window_get_icon_name(this->wnck_window_));
}

GdkPixbuf *Window::get_icon()
{
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
    auto wnck_window = wnck_window_get_transient(this->wnck_window_);
    return WindowManager::get_instance()->lookup_window(wnck_window);
}

std::string Window::get_class_group_name()
{
    RET_WRAP_NULL(wnck_window_get_class_group_name(this->wnck_window_));
}

void Window::keyboard_move()
{
    g_return_if_fail(this->wnck_window_ != nullptr);
    wnck_window_keyboard_move(this->wnck_window_);
}

bool Window::needs_attention()
{
    return wnck_window_needs_attention(this->wnck_window_);
}

std::string Window::get_class_instance_name()
{
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

bool Window::is_skip_pager()
{
    return wnck_window_is_skip_pager(this->wnck_window_);
}

bool Window::is_skip_taskbar()
{
    return wnck_window_is_skip_tasklist(this->wnck_window_);
}

void Window::activate(uint32_t timestamp)
{
    SETTINGS_PROFILE("xid: %" PRIu64 ", timestamp: %d.", this->get_xid(), timestamp);

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
    SETTINGS_PROFILE("xid: %" PRIu64 ", timestamp: %d.", this->get_xid(), timestamp);
    wnck_window_unminimize(this->wnck_window_, timestamp);
}

bool Window::is_minimized()
{
    return wnck_window_is_minimized(this->wnck_window_);
}

void Window::minimize()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    wnck_window_minimize(this->wnck_window_);
}

bool Window::is_maximized()
{
    return wnck_window_is_maximized(this->wnck_window_);
}

void Window::maximize()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    wnck_window_maximize(this->wnck_window_);
}

void Window::unmaximize()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    wnck_window_unmaximize(this->wnck_window_);
}

bool Window::is_shaded()
{
    return wnck_window_is_shaded(this->wnck_window_);
}

void Window::pin()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    return wnck_window_pin(this->wnck_window_);
}

void Window::unpin()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    return wnck_window_unpin(this->wnck_window_);
}

void Window::make_above()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    wnck_window_make_above(this->wnck_window_);
}

void Window::make_unabove()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ".", this->get_xid());
    wnck_window_unmake_above(this->wnck_window_);
}

bool Window::is_active()
{
    return wnck_window_is_active(this->wnck_window_);
}

void Window::move_to_workspace(std::shared_ptr<Workspace> workspace)
{
    SETTINGS_PROFILE("xid: %" PRIu64 ", number: %d.",
                     this->get_xid(),
                     workspace ? workspace->get_number() : -1);

    wnck_window_move_to_workspace(this->wnck_window_, workspace->workspace_);
}

uint64_t Window::get_window_group()
{
    return wnck_window_get_group_leader(this->wnck_window_);
}

void Window::close()
{
    SETTINGS_PROFILE("xid: %" PRIu64 ", name: %s.", this->get_xid(), this->get_name().c_str());

    uint64_t now = Glib::DateTime::create_now_local().to_unix();
    wnck_window_close(this->wnck_window_, now);
}

WindowGeometry Window::get_geometry()
{
    int x, y, w, h;
    wnck_window_get_geometry(this->wnck_window_, &x, &y, &w, &h);
    return std::make_tuple(x, y, w, h);
}

WindowGeometry Window::get_client_window_geometry()
{
    int x, y, w, h;
    wnck_window_get_client_window_geometry(this->wnck_window_, &x, &y, &w, &h);
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
    auto wnck_workspace = wnck_window_get_workspace(this->wnck_window_);

    return WorkspaceManager::get_instance()->lookup_workspace(wnck_workspace);
}

void Window::flush_workspace()
{
    SETTINGS_PROFILE("");

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

    LOG_DEBUG("the name of the window %" PRIu64 " is changed. new_name: %s.", window->get_xid(), window->get_name().c_str());

    window->name_changed_.emit();
}

void Window::workspace_changed(WnckWindow *wnck_window, gpointer user_data)
{
    g_return_if_fail(wnck_window != NULL);

    auto xid = wnck_window_get_xid(wnck_window);
    auto name = wnck_window_get_name(wnck_window);

    LOG_DEBUG("the workspace of the window is changed. xid: %" PRIu64 ", name: %s.", xid, name);

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
        LOG_WARNING("cannot find the window for wnck window: %" PRIu64 ".", xid);
    }
}

void Window::geometry_changed(WnckWindow *wnck_window,
                              gpointer user_data)
{
    auto window = WindowManager::get_instance()->lookup_window(wnck_window);
    g_return_if_fail(window);

    LOG_DEBUG("the geometry of the window is changed. xid: %" PRIu64 ", window name: %s.",
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
        LOG_WARNING("the extension composite is unsupported.\n");
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
        wnck_window_pin(this->wnck_window_);
    else
        wnck_window_unpin(this->wnck_window_);
}

bool Window::get_on_visible_workspace()
{
    return wnck_window_is_pinned(this->wnck_window_);
}

bool Window::should_skip_taskbar()
{
    if (is_skip_pager() || is_skip_taskbar())
        return true;
    return get_window_type() != WNCK_WINDOW_NORMAL && get_window_type() != WNCK_WINDOW_DIALOG;
}

void Window::set_icon_geometry(int x, int y, int width, int height)
{
    wnck_window_set_icon_geometry(this->wnck_window_,
                                  x,
                                  y,
                                  width,
                                  height);
}
}  // namespace Kiran
