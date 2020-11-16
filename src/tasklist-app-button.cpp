#include "tasklist-app-button.h"
#include "window-manager.h"
#include <iostream>
#include <algorithm>
#include <gtk/gtkx.h>
#include "tasklist-buttons-container.h"

static bool kiran_app_is_active(const std::shared_ptr<Kiran::App> &app)
{
    auto window_manager = Kiran::WindowManager::get_instance();
    auto active_window = window_manager->get_active_window();

    if (active_window) {
        if (!KiranHelper::window_is_ignored(active_window))
            return active_window->get_app() == app;
    }

    return false;
}

TasklistAppButton::TasklistAppButton(const std::shared_ptr<Kiran::App> &app_, int size_):
    Glib::ObjectBase("KiranTasklistAppButton"),
    indicator_size_property(*this, "indicator-size", G_MAXINT32),
    app(app_),
    context_menu(nullptr),
    applet_size(size_),
    dragging(false)
{
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);

    if (app_->get_taskbar_windows().size() == 0)
        set_tooltip_text(app_->get_locale_name());

    init_drag_and_drop();
    get_style_context()->add_class("kiran-tasklist-button");
}

TasklistAppButton::~TasklistAppButton()
{
    if (context_menu)
        delete context_menu;
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
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        int minimum_height, natural_height;

        get_preferred_height(minimum_height, natural_height);
        get_preferred_width_for_height(natural_height, minimum_width, natural_width);
    } else {
        //使用父控件的高度设置
        minimum_width = natural_width = applet_size;
    }
}

void TasklistAppButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        //使用父控件的高度设置
        minimum_height = natural_height = applet_size;
    } else {
        int minimum_width, natural_width;

        get_preferred_width(minimum_width, natural_width);
        get_preferred_height_for_width(natural_width, minimum_height, natural_height);
    }
}

void TasklistAppButton::get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        natural_width = height + 8;
        minimum_width = height;
    } else {
        minimum_width = natural_width = height - 8;
    }
}

void TasklistAppButton::get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const
{
    if (get_orientation() == Gtk::ORIENTATION_VERTICAL) {
        natural_height = width + 8;
        minimum_height = width;
    } else {
        minimum_height = natural_height = width - 8;
    }
}

void TasklistAppButton::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::Button::on_size_allocate(allocation);

    if (get_orientation() == Gtk::ORIENTATION_VERTICAL) {
        icon_size = allocation.get_width() - 8;

    } else {
        icon_size = allocation.get_height() - 8;
    }

    update_windows_icon_geometry();
}

bool TasklistAppButton::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    Gdk::RGBA indicator_color("#3ca8ea"), active_color("rgba(255, 255, 255, 0.3)");
    int indicator_size = indicator_size_property.get_value();
    int windows_count;
    int scale = get_scale_factor();
    auto context = get_style_context();

    auto app_ = get_app();
    if (!app_) {
        g_warning("%s: app already expired!!\n", __FUNCTION__);
        return false;
    }

    if (dragging) {
        /*
         * 正在拖动过程中，不绘制内容
         */
        return false;
    }

    g_debug("indicator size %d\n", indicator_size);

    allocation = get_allocation();
    context->set_state(get_state_flags());



    /*
     * 从样式中加载活动窗口背景色和指示器颜色
     */
    if (!context->lookup_color("tasklist_app_active_color", active_color)) {
        g_warning("%s: failed to load color 'tasklist_app_active_color'", __func__);
    }

    if (!context->lookup_color("tasklist_app_indicator_color", indicator_color)) {
        g_warning("%s: failed to load color 'tasklist_app_indicator_color'", __func__);
    }

    windows_count = app_->get_taskbar_windows().size();
    if (kiran_app_is_active(app_)) {
        Gdk::Cairo::set_source_rgba(cr, active_color);
        cr->paint();

        //绘制窗口激活状态指示器
        Gdk::Cairo::set_source_rgba(cr, indicator_color);
        cr->rectangle(0,
                      allocation.get_height() - indicator_size,
                      allocation.get_width(),
                      indicator_size);
        cr->fill();

        if (windows_count > 1) {
            /**
             * 绘制分割符，来表明有多个窗口
             */
            cr->set_source_rgba(0.0, 0.0, 0.0, 0.8);
            cr->set_line_width(0.8);
            cr->move_to(allocation.get_width() - 3, 0);
            cr->line_to(allocation.get_width() - 3, allocation.get_height());
            cr->stroke();
        }
    } else {
        context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());

        //绘制已打开窗口指示器
        if (windows_count > 0) {
            int x_offset = 0;

            /*最多绘制4个指示器*/
            if (windows_count > 4)
                windows_count = 4;
            x_offset = (allocation.get_width() - (indicator_size * windows_count + 3 * (windows_count - 1)))/2;
            x_offset += indicator_size/2;

            Gdk::Cairo::set_source_rgba(cr, indicator_color);
            for (int i = 0; i < windows_count; i++) {

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
    GdkEvent *event = nullptr;
    auto app_ = get_app();

    if (!app_) {
        g_warning("%s: app already expired", __FUNCTION__);
        return false;
    }
    event = reinterpret_cast<GdkEvent*>(button_event);
    if (gdk_event_triggers_context_menu(event)) {
        if (context_menu == nullptr) {
            context_menu = new TasklistAppContextMenu(app_);
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                        [this]() -> void {
                            m_signal_context_menu_toggled.emit(false);
                        });
        } else {
            //刷新右键菜单内容，因为收藏夹等选项可能需要更新
            context_menu->refresh();
        }
        context_menu->show_all();
        context_menu->popup_at_pointer(event);
        m_signal_context_menu_toggled.emit(true);
        return true;
    }
    set_state_flags(Gtk::STATE_FLAG_ACTIVE, false);

    return false;
}

bool TasklistAppButton::on_button_release_event(GdkEventButton *button_event UNUSED)
{
    auto app_ = get_app();

    if (!app_) {
        g_warning("%s: app already expired", __FUNCTION__);
        return false;
    }

    set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_ACTIVE, true);
    auto windows_list = app_->get_taskbar_windows();
    if (windows_list.size() == 0) {
        //无已打开窗口，打开新的应用窗口(仅针对常驻任务栏应用)
        app_->launch();
        return true;
    }

    if (windows_list.size() == 1)
    {
        /* 当前应用只有一个窗口，不需要显示预览窗口，返回true */
        auto first = windows_list.at(0);
        first->activate(0);
        return false;
    }
    return false;
}

bool TasklistAppButton::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (get_context_menu_opened())
        return true;

    return Gtk::Button::on_enter_notify_event(crossing_event);
}

bool TasklistAppButton::on_leave_notify_event(GdkEventCrossing *crossing_event UNUSED)
{
    if (get_context_menu_opened())
        return true;
    return Gtk::Button::on_leave_notify_event(crossing_event);
}

void TasklistAppButton::on_drag_begin(const Glib::RefPtr<Gdk::DragContext> &context)
{
    auto pixbuf = get_app_icon_pixbuf();

    /*
     * 将应用图标作为拖动的图标，鼠标位于图标中心位置
     */
    context->set_icon(pixbuf, pixbuf->get_width()/2, pixbuf->get_height()/2);
    dragging = true;
    queue_draw();
}

void TasklistAppButton::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &context,
                                         Gtk::SelectionData &selection_data,
                                         guint info,
                                         guint time)
{
    auto app = get_app();
    if (!app) {
        g_warning("%s: app expired, return nothing");
        selection_data.set(8, nullptr, 0);
    }

    /* 传递内容为应用的desktop ID */
    const char *raw_data = app->get_desktop_id().c_str();
    selection_data.set(8, (const guchar*)raw_data, strlen(raw_data));
}

void TasklistAppButton::on_drag_data_delete(const Glib::RefPtr<Gdk::DragContext> &context)
{
    /*
     * 拖动结束，恢复显示按钮图标
     */
    dragging = false;
    set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_ACTIVE, true);
    queue_draw();
}

void TasklistAppButton::on_drag_end(const Glib::RefPtr<Gdk::DragContext> &context)
{
    /*
     * 拖动结束，恢复显示按钮图标
     */
    dragging = false;
    set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_ACTIVE, true);
    queue_draw();
}

const std::shared_ptr<Kiran::App> TasklistAppButton::get_app()
{
    return app.lock();
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
    const TasklistButtonsContainer *parent = static_cast<const TasklistButtonsContainer*>(get_parent());

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

    if (!get_has_window()) {
        origin_x += allocation.get_x();
        origin_y += allocation.get_y();
    }

    for (auto window: get_app()->get_taskbar_windows()) {
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
    auto app_ = app.lock();

    if (!app_)
        return pixbuf;

    scale = get_scale_factor();
    try {
        auto gicon = app_->get_icon();

        if (gicon)
        {
            //将应用图标转换为pixbuf
            std::string icon_desc = gicon->to_string();

            if (icon_desc[0] == '/') {
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
        } else {
            /*
             * 无法获取到应用图标的情况下，使用应用第一个已打开窗口的图标作为应用图标
             */
            auto windows = app_->get_taskbar_windows();

            if (windows.size() > 0) {
                pixbuf = Glib::wrap(windows.front()->get_icon(), true);
                pixbuf = pixbuf->scale_simple(icon_size * scale, icon_size * scale, Gdk::INTERP_BILINEAR);
            }
        }
    } catch (const Glib::Error &e) {
        g_warning("Error occured while trying to load app icon: %s", e.what().c_str());
        pixbuf.clear();
    }

    if (!pixbuf)
    {
        //未能找到对应应用的图标，使用内置的默认图标
        pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-tasklist/icon/executable",
                                                   icon_size * scale,
                                                   icon_size * scale);
    }

    return pixbuf;
}

void TasklistAppButton::init_drag_and_drop()
{
    Gtk::TargetEntry entry("binary/app-id", Gtk::TARGET_SAME_APP);
    std::vector<Gtk::TargetEntry> targets;

    targets.push_back(entry);
    drag_source_set(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
}
