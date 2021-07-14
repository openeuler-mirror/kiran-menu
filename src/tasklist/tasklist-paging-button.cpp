/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

#include "tasklist-paging-button.h"
#include "global.h"
#include "lib/base.h"

TasklistPagingButton::TasklistPagingButton(MatePanelApplet *applet_) : applet(applet_),
                                                                       drag_triggered(false),
                                                                       icon_resource(""),
                                                                       icon_size(16)
{
    /*
     * 当拖动的应用按钮经过分页按钮上方时，触发页面跳转
     */
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("binary/app-id"));
    drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);

    get_style_context()->add_class("tasklist-arrow-button");
}

void TasklistPagingButton::set_icon_image(const Glib::ustring icon_resource_, int icon_size_)
{
    if (icon_resource == icon_resource_ && icon_size == icon_size_)
        return;

    icon_resource = icon_resource_;
    icon_size = icon_size_;
    queue_draw();
}

bool TasklistPagingButton::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;
    cairo_surface_t *surface = NULL;
    Gtk::Allocation allocation = get_allocation();
    int scale_factor = get_scale_factor();
    auto context = get_style_context();

    context->render_background(cr,
                               0,
                               0,
                               allocation.get_width(),
                               allocation.get_height());

    try
    {
        icon_pixbuf = Gdk::Pixbuf::create_from_resource(icon_resource,
                                                        icon_size * scale_factor,
                                                        icon_size * scale_factor);

        surface = gdk_cairo_surface_create_from_pixbuf(icon_pixbuf->gobj(),
                                                       scale_factor,
                                                       NULL);

        if (!get_sensitive())
        {
            /* 禁用情况下绘制暗色 */
            cairo_t *cr = cairo_create(surface);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_paint_with_alpha(cr, 0.5);
            cairo_destroy(cr);
        }

        gtk_render_icon_surface(context->gobj(),
                                cr->cobj(),
                                surface,
                                (allocation.get_width() - icon_size) / 2,
                                (allocation.get_height() - icon_size) / 2);
        cairo_surface_destroy(surface);
    }
    catch (const Gio::ResourceError &e)
    {
        KLOG_WARNING("Error occurred while trying to load resource '%s': %s",
                     icon_resource.c_str(),
                     e.what().c_str());
    }
    catch (const Gdk::PixbufError &e)
    {
        KLOG_WARNING("Error occurred while creating pixbuf for resource '%s': %s",
                     icon_resource.c_str(),
                     e.what().c_str());
    }

    return false;
}

void TasklistPagingButton::on_clicked()
{
    /*
     * 获取输入焦点，当鼠标离开任务栏后当前窗口变化时，任务栏应用按钮会自动显示对应的状态
     */
    mate_panel_applet_request_focus(applet, gtk_get_current_event_time());
    grab_focus();

    Gtk::Button::on_clicked();
}

bool TasklistPagingButton::on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    if (!drag_triggered)
    {
        clicked();
        drag_triggered = true;
    }
    context->drag_refuse(time);
    return true;
}

void TasklistPagingButton::on_drag_leave(const Glib::RefPtr<Gdk::DragContext> &context, guint time)
{
    drag_triggered = false;
}
