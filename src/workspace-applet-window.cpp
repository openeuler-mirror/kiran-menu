#include "workspace-applet-window.h"
#include "window-manager.h"
#include "workspace-manager.h"
#include "workspace-thumbnail.h"
#include "workspace-window-thumbnail.h"
#include "workspace-windows-overview.h"
#include "kiranhelper.h"
#include <X11/Xlib.h>

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-bg.h>
#include <gdk/gdkkeysyms.h>

WorkspaceAppletWindow::WorkspaceAppletWindow():
    selected_workspace(-1)
{
    builder = Gtk::Builder::create_from_resource("/kiran-applet/ui/workspace-applet-window");

    builder->get_widget<Gtk::Box>("content-layout", content_layout);
    builder->get_widget<Gtk::Box>("left-layout", left_layout);
    builder->get_widget<Gtk::Box>("right-layout", right_layout);

    right_layout->add(overview);
    content_layout->reparent(*this);
    content_layout->show_all();

    set_app_paintable(true);
    set_skip_pager_hint(true);
    set_skip_taskbar_hint(true);
    set_decorated(false);
}

void WorkspaceAppletWindow::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{
    Gdk::Rectangle geometry;
    auto monitor = Gdk::Display::get_default()->get_primary_monitor();

    monitor->get_geometry(geometry);
    min_width = natural_width = geometry.get_width();
}

void WorkspaceAppletWindow::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{
    Gdk::Rectangle geometry;
    auto monitor = Gdk::Display::get_default()->get_primary_monitor();

    monitor->get_geometry(geometry);
    min_height = natural_height = geometry.get_height();
}

void WorkspaceAppletWindow::on_realize()
{
    Glib::RefPtr<Gdk::Visual> rgba_visual;
    auto screen = Gdk::Screen::get_default();

    /*设置窗口的Visual为RGBA visual，确保窗口背景透明度可以正常绘制 */
    rgba_visual = screen->get_rgba_visual();
    gtk_widget_set_visual(reinterpret_cast<GtkWidget*>(gobj()), rgba_visual->gobj());

    Gtk::Window::on_realize();
}

bool WorkspaceAppletWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    cairo_surface_t *surface;
    MateBG *bg = mate_bg_new();

    //先绘制桌面背景图
    mate_bg_load_from_preferences(bg);
    surface = mate_bg_create_surface(bg, this->get_window()->gobj(), get_width(), get_height(), FALSE);
    cairo_set_source_surface(cr->cobj(), surface, 0, 0);
    cairo_paint(cr->cobj());

    g_object_unref(bg);
    cairo_surface_flush(surface);
    cairo_surface_destroy(surface);

    //Draw dark shadow
    cairo_set_source_rgba(cr->cobj(), 0, 0, 0, 0.2);
    cairo_paint(cr->cobj());

    propagate_draw(*content_layout, cr);

    return false;
}

bool WorkspaceAppletWindow::on_key_press_event(GdkEventKey *event)
{
    if (event->keyval == GDK_KEY_Escape) {
        /**
         * Close window if ESC key pressed
         */
        if (selected_workspace >= 0) {
            auto workspace = Kiran::WorkspaceManager::get_instance()->get_workspace(selected_workspace);

            if (workspace)
                workspace->activate(0);
            else
                g_warning("workspace with number %d not found", selected_workspace);
        }
        hide();
        return true;
    }
    return Gtk::Window::on_key_press_event(event);
}

void WorkspaceAppletWindow::update_ui()
{
    ws_list.empty();

    KiranHelper::remove_all_for_container(*left_layout);
    auto active_ws = Kiran::WorkspaceManager::get_instance()->get_active_workspace();

    for (auto workspace: Kiran::WorkspaceManager::get_instance()->get_workspaces())
    {
        auto thumbnail_area = Gtk::manage(new WorkspaceThumbnail(workspace));

        left_layout->pack_start(*thumbnail_area, Gtk::PACK_SHRINK);
        ws_list.insert(std::make_pair(workspace->get_number(), thumbnail_area));

        //点击工作区时切换右侧的窗口预览
        thumbnail_area->signal_selected().connect([this](int ws_number) -> void {
            for (auto data: this->ws_list){
                auto num = data.first;
                auto area = data.second;
                if (num != ws_number)
                    area->set_current(false);
                else {
                    area->set_current(true);

                    //通知窗口列表预览控件重绘
                    auto workspace = area->get_workspace();
                    overview.set_workspace(workspace);
                    selected_workspace = workspace->get_number();
                }
            }
        });
        //默认显示当前工作区的窗口预览
        if (workspace == active_ws)
            thumbnail_area->signal_selected().emit(workspace->get_number());
        thumbnail_area->show_all();

        workspace->signal_windows_changes().connect(
                    sigc::bind<int>(sigc::mem_fun(*this, &WorkspaceAppletWindow::update_workspace), workspace->get_number()));
    }
}

void WorkspaceAppletWindow::on_map()
{
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &WorkspaceAppletWindow::update_ui));
    Gtk::Window::on_map();
    set_on_all_workspaces();
}

void WorkspaceAppletWindow::update_workspace(int workspace_num)
{
    WorkspaceThumbnail *thumbnail = nullptr;
    auto iter = ws_list.find(workspace_num);
    if (G_UNLIKELY(iter == ws_list.end())) {
        g_warning("workspace with num %d not found in cached thumbnails\n", workspace_num);
        return;
    }

    thumbnail = iter->second;
    thumbnail->queue_draw();

    /*如果窗口所属的工作区是当前显示的工作区，那么重绘右侧的窗口预览图*/
    if (thumbnail->get_is_current()) {
        //通知窗口列表预览控件重绘
        auto workspace = thumbnail->get_workspace();
        overview.set_workspace(workspace);
    }
}

void WorkspaceAppletWindow::set_on_all_workspaces()
{
    XEvent ev;
    GdkDisplay *display = get_display()->gobj();
    XID xid = GDK_WINDOW_XID(get_window()->gobj());
    Display *xdisplay = GDK_DISPLAY_XDISPLAY(display);
    XID root = GDK_WINDOW_XID(get_screen()->get_root_window()->gobj());

    ev.xclient.type = ClientMessage;
    ev.xclient.serial = 0;
    ev.xclient.send_event = True;
    ev.xclient.display = xdisplay;
    ev.xclient.window = xid;
    ev.xclient.message_type = XInternAtom(xdisplay, "_NET_WM_DESKTOP", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 0xffffffff;  //all desktops
    ev.xclient.data.l[1] = 2;           //pager
    ev.xclient.data.l[2] = 0;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;

    gdk_x11_display_error_trap_push(display);

    XSendEvent(xdisplay,
               root,
               False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               &ev);
    gdk_x11_display_error_trap_pop_ignored(display);
}
