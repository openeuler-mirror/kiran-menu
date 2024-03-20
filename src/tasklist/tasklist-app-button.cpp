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

#include "tasklist-app-button.h"
#include <gtk/gtkx.h>
#include <algorithm>
#include <iostream>
#include "kiran-helper.h"
#include "lib/base.h"
#include "tasklist-buttons-container.h"
#include "window-manager.h"
#include "workspace-manager.h"

TasklistAppButton::TasklistAppButton(const std::shared_ptr<Kiran::App> &app_, int size_) : Glib::ObjectBase("KiranTasklistAppButton"),
                                                                                           context_menu(nullptr),
                                                                                           indicator_size_property(*this, "indicator-size", G_MAXINT32),
                                                                                           applet_size(size_),
                                                                                           app(nullptr),
                                                                                           state(APP_BUTTON_STATE_NORMAL)
{
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK);

    get_style_context()->add_class("kiran-tasklist-button");
    get_style_context()->add_class("flat");

    set_app(app_);

    auto window_manager = Kiran::WindowManager::get_instance();
    window_manager->signal_window_opened().connect(
        sigc::mem_fun(*this, &TasklistAppButton::on_window_opened));

    /* 响应拖放操作 */
    gesture = Gtk::GestureDrag::create(*this);
    gesture->set_propagation_phase(Gtk::PHASE_CAPTURE);

    gesture->signal_drag_update().connect(
        sigc::mem_fun(*this, &TasklistAppButton::on_gesture_drag_update));

    gesture->signal_drag_end().connect(
        sigc::mem_fun(*this, &TasklistAppButton::on_gesture_drag_end));

    init_dnd();
}

TasklistAppButton::~TasklistAppButton()
{
    if (context_menu)
        delete context_menu;

    /* 断开原有的状态监控信号 */
    draw_attention_flicker.disconnect();
    draw_attention_normal.disconnect();
    for (auto connection : windows_state_handlers)
        connection.disconnect();
    windows_state_handlers.clear();
}

void TasklistAppButton::set_app(const std::shared_ptr<Kiran::App> &app_)
{
    if (app)
        KLOG_DEBUG("update app for button '%s'", app->get_desktop_id().c_str());

    app = app_;

    auto windows_list = KiranHelper::get_taskbar_windows(app);
    if (windows_list.size() == 0)
        set_tooltip_text(app->get_locale_name());

    /* 断开原有的窗口状态监控信号 */
    for (auto connection : windows_state_handlers)
        connection.disconnect();
    windows_state_handlers.clear();

    for (auto window : windows_list)
    {
        auto connection = window->signal_state_changed().connect(
            sigc::mem_fun(*this, &TasklistAppButton::on_windows_state_changed));

        windows_state_handlers.push_back(connection);
    }

    /* 重新绘制按钮 */
    if (get_realized())
        queue_draw();
}

sigc::signal<void, int, int> TasklistAppButton::signal_drag_update()
{
    return m_signal_drag_update;
}

sigc::signal<void> TasklistAppButton::signal_drag_end()
{
    return m_signal_drag_end;
}

void TasklistAppButton::set_size(int size)
{
    applet_size = size;
    queue_resize();
}

Gtk::SizeRequestMode TasklistAppButton::get_request_mode_vfunc() const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    else
        return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void TasklistAppButton::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
    {
        int minimum_height, natural_height;

        get_preferred_height(minimum_height, natural_height);
        get_preferred_width_for_height(natural_height, minimum_width, natural_width);
    }
    else
    {
        //使用父控件的高度设置
        minimum_width = natural_width = applet_size;
    }
}

void TasklistAppButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
    {
        //使用父控件的高度设置
        minimum_height = natural_height = applet_size;
    }
    else
    {
        int minimum_width, natural_width;

        get_preferred_width(minimum_width, natural_width);
        get_preferred_height_for_width(natural_width, minimum_height, natural_height);
    }
}

void TasklistAppButton::get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
    {
        natural_width = height + 8;
        minimum_width = height;
    }
    else
    {
        minimum_width = natural_width = height - 8;
    }
}

void TasklistAppButton::get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const
{
    if (get_orientation() == Gtk::ORIENTATION_VERTICAL)
    {
        natural_height = width + 8;
        minimum_height = width;
    }
    else
    {
        minimum_height = natural_height = width - 8;
    }
}

void TasklistAppButton::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::Button::on_size_allocate(allocation);

    if (get_orientation() == Gtk::ORIENTATION_VERTICAL)
    {
        icon_size = allocation.get_width() - 8;
    }
    else
    {
        icon_size = allocation.get_height() - 8;
    }

    update_windows_icon_geometry();
}

bool TasklistAppButton::on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    Gdk::RGBA indicator_color("#3ca8ea"), active_color("rgba(255, 255, 255, 0.3)");
    int indicator_size = indicator_size_property.get_value();
    int scale = get_scale_factor();
    auto context = get_style_context();

    auto app = get_app();
    if (!app)
    {
        KLOG_WARNING("%s: app already expired!!\n", __FUNCTION__);
        return false;
    }

    KLOG_DEBUG("indicator size %d\n", indicator_size);

    allocation = get_allocation();
    context->set_state(get_state_flags());

    /*
     * 从样式中加载活动窗口背景色和指示器颜色
     */
    if (!context->lookup_color("tasklist_app_active_color", active_color))
    {
        KLOG_WARNING("failed to load color 'tasklist_app_active_color'");
    }

    if (!context->lookup_color("tasklist_app_indicator_color", indicator_color))
    {
        KLOG_WARNING("failed to load color 'tasklist_app_indicator_color'");
    }

    auto windows_count = KiranHelper::get_taskbar_windows(app).size();
    auto active_workspace = Kiran::WorkspaceManager::get_instance()->get_active_workspace();

    KLOG_DEBUG("App '%s' has %d windows, in workspace %d",
               app->get_name().c_str(),
               windows_count,
               active_workspace ? active_workspace->get_number() : -1);

    if (app->is_active())
    {
        Gdk::Cairo::set_source_rgba(cr, active_color);
        cr->paint();

        draw_attentions(cr);

        //绘制窗口激活状态指示器
        Gdk::Cairo::set_source_rgba(cr, indicator_color);
        cr->rectangle(0,
                      allocation.get_height() - indicator_size,
                      allocation.get_width(),
                      indicator_size);
        cr->fill();

        if (windows_count > 1)
        {
            /**
             * 绘制分割符，来表明有多个窗口
             */
            cr->set_source_rgba(0.0, 0.0, 0.0, 0.8);
            cr->set_line_width(0.8);
            cr->move_to(allocation.get_width() - 3, 0);
            cr->line_to(allocation.get_width() - 3, allocation.get_height());
            cr->stroke();
        }
    }
    else
    {
        context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());

        draw_attentions(cr);

        //绘制已打开窗口指示器
        if (windows_count > 0)
        {
            int x_offset = 0;

            /*最多绘制4个指示器*/
            if (windows_count > 4)
                windows_count = 4;
            x_offset = (allocation.get_width() - (indicator_size * windows_count + 3 * (windows_count - 1))) / 2;
            x_offset += indicator_size / 2;

            Gdk::Cairo::set_source_rgba(cr, indicator_color);
            for (int i = 0; i < windows_count; i++)
            {
                cr->arc(x_offset + i * (indicator_size + 3),
                        allocation.get_height() - indicator_size / 2.0 - 1,
                        indicator_size / 2.0,
                        0,
                        2 * M_PI);
                cr->fill();
            }
        }
    }

    /*
     * 绘制应用图标
     */
    cr->save();

    pixbuf = get_app_icon_pixbuf();
    cr->scale(1.0 / scale, 1.0 / scale);
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                  (allocation.get_width() - icon_size) / 2.0 * scale,
                                  (allocation.get_height() - icon_size - 4) / 2.0 * scale);
    cr->paint();
    cr->restore();

    return false;
}

bool TasklistAppButton::on_button_press_event(GdkEventButton *button_event)
{
    GdkEvent *event = gtk_get_current_event();
    auto app_ = get_app();

    KLOG_WARNING("button pressed");
    pressed = true;

    if (!app_)
    {
        KLOG_WARNING("%s: app already expired", __FUNCTION__);
        return false;
    }

    if (gdk_event_triggers_context_menu(event))
    {
        if (context_menu == nullptr)
        {
            context_menu = new TasklistAppContextMenu(app_);
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                [this]() -> void {
                    m_signal_context_menu_toggled.emit(false);
                });
        }
        else
        {
            //刷新右键菜单内容，因为收藏夹等选项可能需要更新
            context_menu->refresh();
        }
        context_menu->show_all();
        context_menu->popup_at_pointer(event);
        m_signal_context_menu_toggled.emit(true);
        return true;
    }

    return Gtk::Button::on_button_press_event(button_event);
}

void TasklistAppButton::on_clicked()
{
    auto app_ = get_app();

    Gtk::Button::on_clicked();

    if (!pressed)
        return;

    if (!app_)
    {
        KLOG_WARNING("%s: app already expired", __FUNCTION__);
        return;
    }

    auto windows_list = KiranHelper::get_taskbar_windows(app_);
    if (windows_list.size() == 0)
    {
        //无已打开窗口，打开新的应用窗口(仅针对常驻任务栏应用)
        app_->launch();
        return;
    }

    if (windows_list.size() == 1)
    {
        /* 当前应用只有一个窗口，不需要显示预览窗口，返回true */
        auto first = windows_list.at(0);
        first->activate(0);
    }
}

bool TasklistAppButton::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (get_context_menu_opened())
        return true;

    return Gtk::Button::on_enter_notify_event(crossing_event);
}

void TasklistAppButton::on_map()
{
    on_windows_state_changed();
    Gtk::Button::on_map();
}

void TasklistAppButton::on_gesture_drag_update(double x, double y)
{
    Gtk::Allocation allocation;

    if (ABS(x) < 10 && ABS(y) < 10)
    {
        return;
    }

    /* 设置拖动过程中的光标样式 */
    auto cursor = Gdk::Cursor::create(get_display(), "move");
    get_window()->set_cursor(cursor);

    /* 清除pressed标识，确保拖动释放后不会触发clicked事件 */
    pressed = false;
    allocation = get_allocation();
    signal_drag_update().emit(allocation.get_x() + x, allocation.get_y() + y);
}

void TasklistAppButton::on_gesture_drag_end(double x, double y)
{
    /* 恢复光标样式 */
    auto cursor = Gdk::Cursor::create(get_display(), "default");

    get_window()->set_cursor(cursor);
    signal_drag_end().emit();
}

void TasklistAppButton::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection_data, guint info, guint time)
{
#ifdef DEBUG
    for (auto uri : selection_data.get_uris())
    {
        KLOG_WARNING("got uri '%s'", uri.c_str());
    }
#endif

    auto app = get_app();
    if (app)
        app->launch_uris(selection_data.get_uris());
}

void TasklistAppButton::on_window_opened(const std::shared_ptr<Kiran::Window> &window)
{
    if (window && window->get_app() == get_app())
        window->signal_state_changed().connect(
            sigc::mem_fun(*this, &TasklistAppButton::on_windows_state_changed));
}

const std::shared_ptr<Kiran::App> TasklistAppButton::get_app()
{
    return app;
}

bool TasklistAppButton::get_context_menu_opened()
{
    return context_menu != nullptr && context_menu->is_visible();
}

sigc::signal<void, bool> TasklistAppButton::signal_context_menu_toggled()
{
    return m_signal_context_menu_toggled;
}

Gtk::Orientation TasklistAppButton::get_orientation() const
{
    const TasklistButtonsContainer *parent = static_cast<const TasklistButtonsContainer *>(get_parent());

    //使用父控件容器的排列方向
    return parent->get_orientation();
}

void TasklistAppButton::update_windows_icon_geometry()
{
    int origin_x, origin_y;
    int scale_factor;
    Gtk::Allocation allocation;

    if (!get_realized())
        return;

    scale_factor = get_scale_factor();
    allocation = get_allocation();
    get_window()->get_origin(origin_x, origin_y);

    if (!get_has_window())
    {
        origin_x += allocation.get_x();
        origin_y += allocation.get_y();
    }

    for (auto window : KiranHelper::get_taskbar_windows(get_app()))
    {
        window->set_icon_geometry(origin_x * scale_factor,
                                  origin_y * scale_factor,
                                  allocation.get_width() * scale_factor,
                                  allocation.get_height() * scale_factor);
    }
}

Glib::RefPtr<Gdk::Pixbuf> TasklistAppButton::get_app_icon_pixbuf()
{
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    int scale;

    if (!app)
        return pixbuf;

    scale = get_scale_factor();
    try
    {
        auto gicon = app->get_icon();

        if (gicon)
        {
            //将应用图标转换为pixbuf
            std::string icon_desc = gicon->to_string();

            if (icon_desc[0] == '/')
            {
                pixbuf = Gdk::Pixbuf::create_from_file(icon_desc, icon_size * scale, icon_size * scale);
            }
            else
            {
                auto icon_theme = Gtk::IconTheme::get_default();
                pixbuf = icon_theme->load_icon(gicon->to_string(),
                                               icon_size,
                                               scale,
                                               Gtk::ICON_LOOKUP_FORCE_SIZE);
            }
        }
        else
        {
            /*
             * 无法获取到应用图标的情况下，使用应用第一个已打开窗口的图标作为应用图标
             */
            auto windows = KiranHelper::get_taskbar_windows(app);

            if (windows.size() > 0)
            {
                pixbuf = Glib::wrap(windows.front()->get_icon(), true);
                pixbuf = pixbuf->scale_simple(icon_size * scale, icon_size * scale, Gdk::INTERP_BILINEAR);
            }
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Error occured while trying to load app icon: %s", e.what().c_str());
        pixbuf.clear();
    }

    if (!pixbuf)
    {
        //未能找到对应应用的图标，使用内置的默认图标
        // pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-tasklist/icon/executable",
        //                                            icon_size * scale,
        //                                            icon_size * scale);

        pixbuf = Gtk::IconTheme::get_default()->load_icon("kiran-tasklist-executable", icon_size, scale);
    }

    return pixbuf;
}

void TasklistAppButton::on_windows_state_changed()
{
    if (needs_attention())
    {
        if (!draw_attention_flicker.connected())
        {
            state = APP_BUTTON_STATE_FLICKER;
            /*
             * 让应用按钮每500ms闪烁一次，提示用户注意
             */
            draw_attention_flicker = Glib::signal_timeout().connect(
                sigc::bind_return<bool>(sigc::mem_fun(*this, &Gtk::Widget::queue_draw), true),
                400);

            /*
             * 闪烁5秒后停止，并绘制提示颜色
             */
            draw_attention_normal = Glib::signal_timeout().connect(
                [this]() -> bool {
                    draw_attention_flicker.disconnect();
                    if (needs_attention())
                        state = APP_BUTTON_STATE_ATTENTION;
                    else
                        state = APP_BUTTON_STATE_NORMAL;

                    queue_draw();
                    return false;
                },
                2800);
        }
    }
    else
    {
        state = APP_BUTTON_STATE_NORMAL;
        if (draw_attention_flicker.connected())
            draw_attention_flicker.disconnect();

        if (draw_attention_normal.connected())
            draw_attention_normal.disconnect();
        queue_draw();
    }
}

void TasklistAppButton::draw_attentions(const Cairo::RefPtr<Cairo::Context> &cr)
{
    static bool hilight = true;
    Gdk::RGBA attention_color("red");

    /*
     * 从样式表中加载提示颜色
     */
    if (!get_style_context()->lookup_color("tasklist_attention_color", attention_color))
    {
        KLOG_WARNING("Failed to load attention-color from style");
    }

    attention_color.set_alpha(1.0);
    cr->save();
    if (state == APP_BUTTON_STATE_FLICKER)
    {
        if (hilight)
        {
            Gdk::Cairo::set_source_rgba(cr, attention_color);
            cr->paint();
        }

        hilight = !hilight;
    }
    else if (state == APP_BUTTON_STATE_ATTENTION)
    {
        Gdk::Cairo::set_source_rgba(cr, attention_color);
        cr->paint();
    }
    cr->restore();
}

void TasklistAppButton::init_dnd()
{
    std::vector<Gtk::TargetEntry> targets;

    targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TARGET_OTHER_APP));
    drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
}

bool TasklistAppButton::needs_attention()
{
    auto app_ = get_app();
    if (!app_)
        return false;

    for (auto window : KiranHelper::get_taskbar_windows(app_))
    {
        if (window->needs_attention())
            return true;
    }

    return false;
}
