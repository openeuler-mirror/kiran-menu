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

#include "workspace-thumbnail.h"
#include <cairomm/xlib_surface.h>
#include <gtk/gtkx.h>
#include "global.h"
#include "lib/base.h"
#include "lib/common.h"
#include "window-manager.h"
#include "workspace-manager.h"

WorkspaceThumbnail::WorkspaceThumbnail(KiranWorkspacePointer &workspace_) : workspace(workspace_),
                                                                            bg_surface(nullptr),
                                                                            border_width(4),
                                                                            drop_check(false),
                                                                            draw_windows(false)
{
    /* 插件设置变化时重绘 */
    applet_settings = Gio::Settings::create(WORKSPACE_SCHEMA);
    draw_windows = applet_settings->get_boolean(WORKSPACE_KEY_DRAW_WINDOWS);
    applet_settings->signal_changed().connect(
        sigc::mem_fun(*this, &WorkspaceThumbnail::on_settings_changed));

    /*屏幕大小变化时重绘背景*/
    Gdk::Screen::get_default()->signal_size_changed().connect(
        sigc::mem_fun(*this, &WorkspaceThumbnail::on_background_changed));

    /* 窗口列表发生变化时重绘缩略图 */
    workspace_->signal_windows_changes().connect(
        sigc::mem_fun(*this, &Gtk::Widget::queue_draw));

    set_vspacing(10);
    set_title(workspace_->get_name());
    set_thumbnail_size(150, 80);
    set_show_icon(false);

    init_drag_and_drop();

    get_style_context()->add_class("workspace-thumbnail");
}

WorkspaceThumbnail::~WorkspaceThumbnail()
{
    if (bg_surface)
    {
        cairo_surface_destroy(bg_surface);
    }
}

void WorkspaceThumbnail::set_selected(bool selected)
{
    if (is_selected() == selected)
        return;

    if (selected)
        set_state_flags(Gtk::STATE_FLAG_SELECTED, false);
    else
        set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_SELECTED, true);

    queue_draw();
}

bool WorkspaceThumbnail::is_selected() const
{
    return get_state_flags() & Gtk::STATE_FLAG_SELECTED;
}

KiranWorkspacePointer WorkspaceThumbnail::get_workspace()
{
    return workspace.lock();
}

void WorkspaceThumbnail::on_background_changed()
{
    reload_bg_surface();
    queue_draw();
}

void WorkspaceThumbnail::init_drag_and_drop()
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::TargetEntry entry("binary/XID", Gtk::TARGET_SAME_APP);

    targets.push_back(entry);
    drag_dest_set(targets, Gtk::DEST_DEFAULT_HIGHLIGHT, Gdk::ACTION_MOVE);
}

bool WorkspaceThumbnail::reload_bg_surface()
{
    Gtk::Allocation allocation;
    double x_scale, y_scale;
    MateBG *bg = nullptr;
    auto screen = get_screen();

    /**
     * 计算X和Y两个方向的缩放比例.
     * 为了应对多显示器扩展显示的情况，这里使用screen的大小
     */
    allocation = get_thumbnail_area()->get_allocation();
    surface_width = allocation.get_width() - 2 * border_width;
    surface_height = allocation.get_height() - 2 * border_width;
    x_scale = surface_width * 1.0 / screen->get_width();
    y_scale = surface_height * 1.0 / screen->get_height();
    surface_scale = MIN(x_scale, y_scale);

    surface_width = static_cast<int>(screen->get_width() * surface_scale);
    surface_height = static_cast<int>(screen->get_height() * surface_scale);

    if (bg_surface)
        cairo_surface_destroy(bg_surface);

    bg = mate_bg_new();
    mate_bg_load_from_preferences(bg);

    if (!get_realized())
        return true;

    bg_surface = mate_bg_create_surface(bg,
                                        get_window()->gobj(),
                                        surface_width,
                                        surface_height,
                                        FALSE);
    g_object_unref(bg);
    return bg_surface != nullptr;
}

bool WorkspaceThumbnail::draw_thumbnail_image(Gtk::Widget *thumbnail_area_, const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto workspace_ = workspace.lock();
    Display *xdisplay = gdk_x11_get_default_xdisplay();
    Gtk::Allocation allocation;
    std::vector<Glib::RefPtr<Gdk::Window>> windows_stack;
    Cairo::ImageSurface *surface = nullptr;
    auto screen = get_screen();
    double surface_offset_x, surface_offset_y;
    int scale_factor = get_scale_factor();

    if (!workspace_)
        return false;

    allocation = thumbnail_area_->get_allocation();
    // 调用mate-desktop接口来获取桌面背景图片, 省去缩放等相关操作
    if (!bg_surface)
        reload_bg_surface();

    // 背景图片居中显示
    surface_offset_x = (allocation.get_width() - surface_width) / 2.0;
    surface_offset_y = (allocation.get_height() - surface_height) / 2.0;
    surface = new Cairo::ImageSurface(bg_surface, false);
    cr->set_source(Cairo::RefPtr<Cairo::ImageSurface>(surface),
                   surface_offset_x,
                   surface_offset_y);
    cr->paint();

    cr->rectangle(surface_offset_x, surface_offset_y, surface_width, surface_height);
    cr->clip();

    if (draw_windows)
    {
        /*
         * 此处调用GDK接口来按照堆叠顺序获取窗口列表(靠底层的窗口在列表的靠前位置)，然后再进行过滤
         */
        for (auto gdk_window : screen->get_window_stack())
        {
            Kiran::WindowGeometry geometry;
            Window wid = GDK_WINDOW_XID(gdk_window->gobj());
            XWindowAttributes attrs;
            Gdk::Rectangle rect;

            int workspace_id = static_cast<int>(gdk_x11_window_get_desktop(gdk_window->gobj()));
            auto window = Kiran::WindowManager::get_instance()->get_window(wid);
            if (!window)
                continue;
            XGetWindowAttributes(xdisplay, wid, &attrs);

            /*
             *  仅绘制位于当前工作区或设置了所有工作区可见的非最小化窗口
             */
            if (window->is_minimized() || window->get_xid() == GDK_WINDOW_XID(get_toplevel()->get_window()->gobj()) ||
                (workspace_->get_number() != workspace_id && !window->is_pinned()))
                continue;

            if (window->get_window_type() == WNCK_WINDOW_DOCK || window->get_window_type() == WNCK_WINDOW_DESKTOP)
                continue;

            KiranHelper::geometry_to_rect(window->get_geometry(), rect);
            Drawable drawable = window->get_pixmap();

            cr->save();
            cr->translate(surface_offset_x, surface_offset_y);
            cr->scale(surface_scale, surface_scale);

            /*window size from X server know nothing about scale*/
            if (drawable != None)
            {
                auto surface = Cairo::XlibSurface::create(xdisplay,
                                                          drawable,
                                                          attrs.visual,
                                                          attrs.width,
                                                          attrs.height);
                cr->scale(1.0 / scale_factor, 1.0 / scale_factor);
                cr->set_source(surface, rect.get_x(), rect.get_y());
                cr->paint();
            }
            else
            {
                auto icon_pixbuf = Glib::wrap(window->get_icon(), true);
                if (!icon_pixbuf)
                    icon_pixbuf = Gtk::IconTheme::get_default()->load_icon(
                        "x-executable",
                        16,
                        scale_factor,
                        Gtk::ICON_LOOKUP_FORCE_SIZE);
                else
                    icon_pixbuf = icon_pixbuf->scale_simple(16 * scale_factor, 16 * scale_factor, Gdk::INTERP_BILINEAR);

                cr->scale(1.0 / scale_factor, 1.0 / scale_factor);
                cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
                cr->rectangle(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());
                cr->fill();
                cr->restore();
                cr->save();

                if (icon_pixbuf)
                {
                    double icon_offset_x, icon_offset_y;

                    icon_offset_x = (rect.get_width() * surface_scale - icon_pixbuf->get_width()) / 2.0;
                    icon_offset_y = (rect.get_height() * surface_scale - icon_pixbuf->get_height()) / 2.0;
                    cr->translate(surface_offset_x, surface_offset_y);
                    cr->scale(1.0 / scale_factor, 1.0 / scale_factor);
                    Gdk::Cairo::set_source_pixbuf(cr,
                                                  icon_pixbuf,
                                                  rect.get_x() * surface_scale + icon_offset_x,
                                                  rect.get_y() * surface_scale + icon_offset_y);
                    cr->paint();
                }
            }
            cr->restore();
        }
    }
    cr->reset_clip();
    if (is_selected())
    {
        /* 绘制选中后的边框 */
        Gdk::RGBA color;
        auto context = get_style_context();

        if (context->lookup_color("thumbnail-hover-color", color))
        {
            Gdk::Cairo::set_source_rgba(cr, color);

            cr->set_line_width(border_width);
            cr->rectangle(border_width / 2.0,
                          border_width / 2.0,
                          allocation.get_width() - border_width,
                          allocation.get_height() - border_width);
            cr->stroke();
        }
        else
            KLOG_WARNING("color 'thumbnail-hover-color' not found");
    }
    else
    {
        cr->set_source_rgba(0.0, 0.0, 0.0, 0.3);
        cr->rectangle(border_width,
                      border_width,
                      allocation.get_width() - border_width * 2,
                      allocation.get_height() - border_width * 2);
        cr->fill();
    }

    return false;
}

void WorkspaceThumbnail::on_close_button_clicked()
{
    g_return_if_fail(!workspace.expired());
    auto manager = Kiran::WorkspaceManager::get_instance();

    manager->destroy_workspace(workspace.lock());
}

void WorkspaceThumbnail::on_thumbnail_clicked()
{
    /* NOTHING */
}

void WorkspaceThumbnail::on_settings_changed(const Glib::ustring &key)
{
    if (key == WORKSPACE_KEY_DRAW_WINDOWS)
    {
        bool new_value = applet_settings->get_boolean(WORKSPACE_KEY_DRAW_WINDOWS);
        if (new_value != draw_windows)
        {
            KLOG_DEBUG("key '%s' changed to %d", WORKSPACE_KEY_DRAW_WINDOWS, new_value);
            queue_draw();
        }
    }
}

bool WorkspaceThumbnail::on_button_press_event(GdkEventButton *event)
{
    if (workspace.expired())
    {
        KLOG_WARNING("workspace already expired");
        return false;
    }

    if (event->type == GDK_2BUTTON_PRESS)
    {
        /* 双击时切换到对应的工作区 */
        workspace.lock()->activate(0);
        get_toplevel()->hide();
        return true;
    }
    return KiranThumbnailWidget::on_button_press_event(event);
}

bool WorkspaceThumbnail::on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    KLOG_DEBUG("drag motion");
    drop_check = true;

    drag_get_data(context, "binary/XID", time);
    return true;
}

bool WorkspaceThumbnail::on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    KLOG_DEBUG("drag-drop finished");
    drop_check = false;
    drag_get_data(context, "binary/XID", time);
    return true;
}

void WorkspaceThumbnail::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection, guint info, guint time)
{
    Gdk::DragAction action = context->get_selected_action();
    int length;
    const guchar *data;
    const Window *wid;

    if (!drop_check && action != Gdk::ACTION_MOVE)
    {
        context->drag_finish(false, false, time);
        return;
    }

    data = selection.get_data(length);
    // 传递的数据应该是Window的XID
    if (length < 0 || length != sizeof(Window))
    {
        if (!drop_check)
            context->drag_finish(false, false, time);
        else
            context->drag_refuse(time);
        return;
    }

    wid = reinterpret_cast<const Window *>(data);

    auto window = Kiran::WindowManager::get_instance()->get_window(*wid);
    if (!window)
    {
        KLOG_WARNING("Window with ID 0x%x not found\n", (int)(*wid));
        if (drop_check)
        {
            context->drag_refuse(time);
            return;
        }
        context->drag_finish(false, false, time);
        return;
    }

    if (window->get_workspace() == workspace.lock())
    {
        /* 不允许将窗口移动到其所在的工作区，禁止拖放操作 */
        if (drop_check)
        {
            context->drag_refuse(time);
            KLOG_DEBUG("Workspace of window and the target workspace are the same, drag-drop refused!");
        }
        else
            context->drag_finish(false, false, time);
        return;
    }

    if (!drop_check)
    {
        // 将对应的窗口移动到该工作区
        window->move_to_workspace(workspace.lock());
        context->drag_finish(true, true, time);
    }
    else
    {
        context->drag_status(Gdk::ACTION_MOVE, time);
    }
}
