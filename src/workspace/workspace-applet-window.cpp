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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "workspace-applet-window.h"
#include <X11/Xlib.h>
#include "global.h"
#include "kiran-helper.h"
#include "lib/base.h"
#include "window-manager.h"
#include "workspace-manager.h"
#include "workspace-thumbnail.h"
#include "workspace-window-thumbnail.h"
#include "workspace-windows-overview.h"

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <gdk/gdkkeysyms.h>
#include <libmate-desktop/mate-bg.h>

WorkspaceAppletWindow::WorkspaceAppletWindow() : selected_workspace(-1)
{
    init_ui();

    set_app_paintable(true);
    set_skip_pager_hint(true);
    set_skip_taskbar_hint(true);
    set_decorated(false);
    set_keep_above(true);

    /* 屏幕和显示器变化时调整窗口大小和位置 */
    get_screen()->signal_monitors_changed().connect_notify(
        sigc::mem_fun(*this, &WorkspaceAppletWindow::resize_and_reposition));
    get_screen()->signal_size_changed().connect_notify(
        sigc::mem_fun(*this, &WorkspaceAppletWindow::resize_and_reposition));

    /*
     * 响应工作区数量变化
     */
    auto workspace_manager = Kiran::WorkspaceManager::get_instance();
    workspace_manager->signal_workspace_created().connect(
        sigc::mem_fun(*this, &WorkspaceAppletWindow::on_workspace_created));
    workspace_manager->signal_workspace_destroyed().connect(
        sigc::mem_fun(*this, &WorkspaceAppletWindow::on_workspace_destroyed));

    resize_and_reposition();
    get_style_context()->add_class("workspace-previewer");
}

void WorkspaceAppletWindow::on_realize()
{
    Glib::RefPtr<Gdk::Visual> rgba_visual;
    auto screen = Gdk::Screen::get_default();

    /*设置窗口的Visual为RGBA visual，确保窗口背景透明度可以正常绘制 */
    rgba_visual = screen->get_rgba_visual();
    gtk_widget_set_visual(reinterpret_cast<GtkWidget *>(gobj()), rgba_visual->gobj());

    Gtk::Window::on_realize();
}

bool WorkspaceAppletWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    cairo_surface_t *surface;
    MateBG *bg = mate_bg_new();

    //先绘制桌面背景图
    mate_bg_load_from_preferences(bg);
    surface = mate_bg_create_surface(bg, get_window()->gobj(), get_width(), get_height(), FALSE);
    cairo_set_source_surface(cr->cobj(), surface, 0, 0);
    cairo_paint(cr->cobj());

    g_object_unref(bg);
    cairo_surface_flush(surface);
    cairo_surface_destroy(surface);

    //绘制暗色阴影
    cairo_set_source_rgba(cr->cobj(), 0, 0, 0, 0.2);
    cairo_paint(cr->cobj());

    propagate_draw(*main_layout, cr);

    return false;
}

bool WorkspaceAppletWindow::on_key_press_event(GdkEventKey *event)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        /*
         * 按下ESC键时隐藏窗口
         */
        if (selected_workspace >= 0)
        {
            auto workspace = Kiran::WorkspaceManager::get_instance()->get_workspace(selected_workspace);

            if (workspace)
                workspace->activate(0);
            else
                KLOG_WARNING("workspace with number %d not found", selected_workspace);
        }
        hide();
        return true;
    }
    return Gtk::Window::on_key_press_event(event);
}

bool WorkspaceAppletWindow::on_map_event(GdkEventAny *event)
{
    /* 抓取鼠标和键盘，确保能收取到ESC按键事件 */
    KiranHelper::grab_input(*this);
    return Gtk::Window::on_map_event(event);
}

void WorkspaceAppletWindow::on_unmap()
{
    /* 取消抓取输入鼠标和键盘 */
    KiranHelper::ungrab_input(*this);
    Gtk::Window::on_unmap();
}

void WorkspaceAppletWindow::init_ui()
{
    Gtk::Button *add_button; /* 创建工作区按钮 */

    builder = Gtk::Builder::create_from_resource("/kiran-applet/ui/workspace-applet-window");

    builder->get_widget<Gtk::Box>("content-layout", main_layout);
    builder->get_widget<Gtk::Box>("left-layout", left_layout);
    builder->get_widget<Gtk::Box>("right-layout", right_layout);
    builder->get_widget<Gtk::Button>("add-button", add_button);

    overview_area.set_halign(Gtk::ALIGN_FILL);
    overview_area.set_valign(Gtk::ALIGN_FILL);
    right_layout->add(overview_area);
    main_layout->reparent(*this);
    main_layout->show_all();

    /* 点击“创建工作区"按钮 */
    add_button->signal_clicked().connect(
        []() -> void {
            auto workspace_manager = Kiran::WorkspaceManager::get_instance();
            auto workspace_count = workspace_manager->get_workspaces().size();
            workspace_manager->change_workspace_count(++workspace_count);
        });
}

void WorkspaceAppletWindow::on_workspace_created(std::shared_ptr<Kiran::Workspace> workspace)
{
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &WorkspaceAppletWindow::load_workspaces));
}

void WorkspaceAppletWindow::on_workspace_destroyed(std::shared_ptr<Kiran::Workspace> workspace)
{
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &WorkspaceAppletWindow::load_workspaces));
}

void WorkspaceAppletWindow::load_workspaces()
{
    auto workspace_manager = Kiran::WorkspaceManager::get_instance();
    auto active_workspace = workspace_manager->get_active_workspace();

    /* 清空界面内容和缓存数据 */
    workspaces_table.clear();
    KiranHelper::remove_all_for_container(*left_layout);

    for (auto workspace : workspace_manager->get_workspaces())
    {
        int workspace_no = workspace->get_number();
        auto thumbnail_area = Gtk::make_managed<WorkspaceThumbnail>(workspace);

        thumbnail_area->set_vexpand(false);
        thumbnail_area->set_valign(Gtk::ALIGN_START);
        thumbnail_area->set_margin_top(30);
        left_layout->pack_start(*thumbnail_area, Gtk::PACK_SHRINK);
        workspaces_table.insert(std::make_pair(workspace_no, thumbnail_area));

        //点击工作区时切换右侧的窗口预览
        thumbnail_area->signal_clicked().connect([this, workspace_no]() -> void {
            for (auto data : workspaces_table)
            {
                auto num = data.first;
                auto area = data.second;
                if (num != workspace_no)
                    area->set_selected(false);
                else
                {
                    area->set_selected(true);

                    //通知窗口列表预览控件重绘
                    auto workspace = area->get_workspace();
                    overview_area.set_workspace(workspace);
                    selected_workspace = workspace->get_number();
                }
            }
        });
        //默认显示当前工作区的窗口预览
        if (workspace == active_workspace)
            thumbnail_area->clicked();
        thumbnail_area->show_all();

        workspace->signal_windows_changes().clear();
        workspace->signal_windows_changes().connect(
            sigc::bind<int>(sigc::mem_fun(*this, &WorkspaceAppletWindow::update_workspace), workspace->get_number()));
    }
}

void WorkspaceAppletWindow::on_map()
{
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &WorkspaceAppletWindow::load_workspaces));
    Gtk::Window::on_map();
    set_on_all_workspaces();
}

void WorkspaceAppletWindow::update_workspace(int workspace_num)
{
    WorkspaceThumbnail *thumbnail = nullptr;
    auto iter = workspaces_table.find(workspace_num);
    if (G_UNLIKELY(iter == workspaces_table.end()))
    {
        KLOG_WARNING("workspace with num %d not found in cached thumbnails\n", workspace_num);
        return;
    }

    thumbnail = iter->second;
    thumbnail->queue_draw();

    /*如果窗口所属的工作区是当前显示的工作区，那么重绘右侧的窗口预览图*/
    if (thumbnail->is_selected())
    {
        //通知窗口列表预览控件重绘
        auto workspace = thumbnail->get_workspace();
        overview_area.set_workspace(workspace);
    }
}

void WorkspaceAppletWindow::set_on_all_workspaces()
{
    g_return_if_fail(get_realized() != false);

    /* 在所有工作区都显示 */
    gdk_x11_window_move_to_desktop(get_window()->gobj(), 0xffffffff);
}

void WorkspaceAppletWindow::resize_and_reposition()
{
    Gdk::Rectangle rect;
    auto monitor = get_display()->get_primary_monitor();

    monitor->get_geometry(rect);

    /* 清空缩略图，确保窗口resize的时候，不会受到之前缩略图控件尺寸的影响 */
    overview_area.clear();

    move(rect.get_x(), rect.get_y());
    KLOG_DEBUG("screen size changed to %d x %d, resize and reposition applet window now", rect.get_width(), rect.get_height());
    resize(rect.get_width(), rect.get_height());
}
