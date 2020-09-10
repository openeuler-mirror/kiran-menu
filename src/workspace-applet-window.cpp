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
    applet(nullptr),
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
        if (applet)
            hide();
        else
            close();
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
