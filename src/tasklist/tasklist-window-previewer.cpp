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

#include "tasklist-window-previewer.h"
#include <cairomm/xlib_surface.h>
#include <glib/gi18n.h>
#include <gtk/gtkx.h>
#include <iostream>
#include "lib/base.h"

TasklistWindowPreviewer::TasklistWindowPreviewer(std::shared_ptr<Kiran::Window> &window_) : WindowThumbnailWidget(window_),
                                                                                            context_menu(nullptr),
                                                                                            attention_color("red")
{
    needs_attention = window_->needs_attention();

    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_no_show_all(true);
    set_vspacing(10);

    // 窗口管理器复合状态(composite)打开或关闭时调用
    // 如果窗口管理器混合模式关闭，我们需要隐藏窗口预览图，因为窗口预览图已经无法获取
    signal_composited_changed().connect(sigc::mem_fun(*this, &TasklistWindowPreviewer::on_composite_changed));

    window_state_change = window_->signal_state_changed().connect(
        sigc::mem_fun(*this, &TasklistWindowPreviewer::on_window_state_changed));

    /*
     * 从样式表中加载提示颜色
     */
    if (!get_style_context()->lookup_color("tasklist_attention_color", attention_color))
    {
        KLOG_WARNING("Failed to load attention-color from style");
    }

    settings = Gio::Settings::create(TASKBAR_SCHEMA);
    settings->signal_changed().connect(
        sigc::mem_fun(*this, &TasklistWindowPreviewer::on_settings_changed));
    refresh_layout();
}

TasklistWindowPreviewer::~TasklistWindowPreviewer()
{
    if (context_menu)
        delete context_menu;

    if (window_state_change.connected())
        window_state_change.disconnect();
}

bool TasklistWindowPreviewer::draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    auto window = get_window_();
    int scale_factor = get_scale_factor();

    if (!window)
        return true;

    allocation = snapshot_area->get_allocation();
    try
    {
        double scale_x, scale_y, scale;
        int width, height;
        cairo_surface_t *thumbnail = window->get_thumbnail(width, height);
        if (thumbnail == nullptr)
        {
            // 如果无法获取到窗口截图，同时窗口不可见，那么在预览区域绘制窗口图标
            int pixbuf_width, pixbuf_height;
            Glib::RefPtr<Gdk::Pixbuf> pixbuf;
            GdkPixbuf *c_pixbuf = window->get_icon();
            int icon_size = 64;

            if (!c_pixbuf)
            {
                auto icon_theme = Gtk::IconTheme::get_default();
                pixbuf = icon_theme->load_icon("application-x-executable",
                                               icon_size * scale_factor,
                                               Gtk::ICON_LOOKUP_FORCE_SIZE);
                return false;
            }
            else
            {
                pixbuf = Glib::wrap(c_pixbuf, true);
                pixbuf = pixbuf->scale_simple(icon_size * scale_factor,
                                              icon_size * scale_factor,
                                              Gdk::INTERP_BILINEAR);
            }

            pixbuf_width = pixbuf->get_width();
            pixbuf_height = pixbuf->get_height();

            cr->scale(1.0 / scale_factor, 1.0 / scale_factor);

            Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                          (allocation.get_width() * scale_factor - pixbuf_width) / 2,
                                          (allocation.get_height() * scale_factor - pixbuf_height) / 2);
            cr->paint();
            return false;
        }
        else
        {
            scale_x = allocation.get_width() * scale_factor * 1.0 / width;
            scale_y = allocation.get_height() * scale_factor * 1.0 / height;

            scale = std::min(scale_x, scale_y);
            cr->scale(1.0 / scale_factor, 1.0 / scale_factor);
            cr->translate((allocation.get_width() * scale_factor - width * scale) / 2.0,
                          (allocation.get_height() * scale_factor - height * scale) / 2.0);
            cr->scale(scale, scale);

            cairo_set_source_surface(cr->cobj(), thumbnail, 0, 0);
            cr->paint();
            cairo_surface_destroy(thumbnail);
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Error occured while trying to draw window thumbnail: %s", e.what().c_str());
    }

    return false;
}

void TasklistWindowPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    GtkStateFlags flags;
    GValue value = G_VALUE_INIT;
    GtkWidget *widget = reinterpret_cast<GtkWidget *>(const_cast<GtkButton *>(gobj()));
    auto context = get_style_context()->gobj();

    flags = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_property(context, "min-width", flags, &value);
    minimum_width = g_value_get_int(&value);
    g_value_unset(&value);

    natural_width = minimum_width;
}

bool TasklistWindowPreviewer::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto window = get_window_();

    if (!window)
        return false;

    if (window->needs_attention())
    {
        /* 绘制需要注意的背景色 */
        auto child = get_child();

        Gdk::Cairo::set_source_rgba(cr, attention_color);
        cr->paint();
        propagate_draw(*child, cr);
        return false;
    }

    return WindowThumbnailWidget::on_draw(cr);
}

void TasklistWindowPreviewer::on_thumbnail_clicked()
{
    auto window = get_window_();

    if (window)
        window->activate(0);
    else
        KLOG_WARNING("window already expired!!!");
}

bool TasklistWindowPreviewer::on_button_press_event(GdkEventButton *button_event)
{
    GdkEvent *event;
    auto window = get_window_();

    if (G_UNLIKELY(!window))
        return false;

    event = reinterpret_cast<GdkEvent *>(button_event);
    if (gdk_event_triggers_context_menu(event))
    {
        // show context menu
        if (!context_menu)
        {
            context_menu = new TasklistWindowContextMenu(window);
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                [this]() -> void
                {
                    signal_context_menu_toggled().emit(false);
                });
        }
        else
            context_menu->refresh();

        context_menu->show_all();
        context_menu->popup_at_pointer(event);
        signal_context_menu_toggled().emit(true);
        return false;
    }

    return WindowThumbnailWidget::on_button_press_event(button_event);
}

void TasklistWindowPreviewer::on_window_state_changed()
{
    auto window = get_window_();
    if (!window)
        return;

    if (needs_attention != window->needs_attention())
    {
        needs_attention = window->needs_attention();
        queue_draw();
    }
}

void TasklistWindowPreviewer::on_composite_changed()
{
    refresh_layout();
}

void TasklistWindowPreviewer::on_settings_changed(const Glib::ustring &changed_key)
{
    if (changed_key == TASKBAR_KEY_SIMPLY_WINDOW_PREVIEWER)
    {
        refresh_layout();
    }
}

void TasklistWindowPreviewer::refresh_layout()
{
    bool is_simply_show = false;
    // 混成器不开启时，使用简单预览
    // gsettings 简单预览配置开启时，使用简单预览
    if (!is_composited() || settings->get_boolean(TASKBAR_KEY_SIMPLY_WINDOW_PREVIEWER))
    {
        is_simply_show = true;
    }
    refresh_layout(is_simply_show);

    if (get_realized())
        queue_resize();
}

void TasklistWindowPreviewer::refresh_layout(bool is_simply)
{
    Gtk::Widget *snapshot_area = get_thumbnail_area();

    if (is_simply)
    {
        snapshot_area->hide();
        get_style_context()->remove_class("horizontal");
        get_style_context()->add_class("vertical");
    }
    else
    {
        snapshot_area->show();
        get_style_context()->remove_class("vertical");
        get_style_context()->add_class("horizontal");
    }
}

bool TasklistWindowPreviewer::context_menu_is_opened()
{
    return context_menu != nullptr && context_menu->is_visible();
}

sigc::signal<void, bool> TasklistWindowPreviewer::signal_context_menu_toggled()
{
    return m_signal_context_menu_toggled;
}
