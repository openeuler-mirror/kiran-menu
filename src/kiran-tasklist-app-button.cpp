#include "kiran-tasklist-app-button.h"
#include "window-manager.h"
#include <iostream>
#include <algorithm>

#define ACTIVE_INDICATOR_HEIGHT 4

static bool kiran_app_is_active(const std::shared_ptr<Kiran::App> &app)
{
    auto window_manager = Kiran::WindowManager::get_instance();
    auto active_window = window_manager->get_active_window();

    if (active_window) {
        if (!KiranHelper::window_is_ignored(active_window))
            return active_window->get_app() == app;
    } else
        g_message("no active window found\n");

    return false;
}

KiranTasklistAppButton::KiranTasklistAppButton(const std::shared_ptr<Kiran::App> &app_):
    app(app_),
    menu_opened(false),
    context_menu(nullptr)
{
    add(drawing_area);

    auto context = get_style_context();
    context->add_class("kiran-tasklist-button");
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    set_has_window(true);
    set_above_child(true);
    set_visible_window(true);

    if (app_->get_taskbar_windows().size() == 0)
        set_tooltip_text(app_->get_locale_name());
}

KiranTasklistAppButton::~KiranTasklistAppButton()
{
    if (context_menu)
        delete context_menu;
}

void KiranTasklistAppButton::on_previewer_opened()
{
    if (menu_opened) {
        context_menu->popdown();
        menu_opened = false;
    }
}

Gtk::SizeRequestMode KiranTasklistAppButton::get_request_mode_vfunc() const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    else
        return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void KiranTasklistAppButton::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        int minimum_height, natural_height;

        get_preferred_height(minimum_height, natural_height); 
        get_preferred_width_for_height(natural_height, minimum_width, natural_width);
    } else {
        //使用父控件的高度设置
        get_parent()->get_preferred_width(minimum_width, natural_width);
    }
    g_message("%s: width %d\n", __func__, natural_width);
}

void KiranTasklistAppButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        //使用父控件的高度设置
        get_parent()->get_preferred_height(minimum_height, natural_height);
    } else {
        int minimum_width, natural_width;

        get_preferred_width(minimum_width, natural_width);
        get_preferred_height_for_width(natural_width, minimum_height, natural_height);
    }
    g_message("%s: height %d\n", __func__, natural_height);
}

void KiranTasklistAppButton::get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const
{
    minimum_width = natural_width = height + 8;
}

void KiranTasklistAppButton::get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const
{
    minimum_height = natural_height = width + 8;
}

void KiranTasklistAppButton::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::EventBox::on_size_allocate(allocation);

    if (get_orientation() == Gtk::ORIENTATION_VERTICAL) {
        icon_size = allocation.get_width() - 8;

    } else {
        icon_size = allocation.get_height() - 8;
    }
}

bool KiranTasklistAppButton::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    auto context = get_style_context();
    int scale = get_scale_factor();
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    auto app_ = get_app();
    Gdk::RGBA indicator_color("#3ca8ea"), active_color("rgba(255, 255, 255, 0.3)");

    if (!app_) {
        g_warning("%s: app already expired!!\n", __FUNCTION__);
        return false;
    }

    allocation = get_allocation();
    context->set_state(get_state_flags());

    /**
     * 从样式中加载活动窗口背景色和指示器颜色
     */
    if (!context->lookup_color("tasklist_app_active_color", active_color)) {
        g_warning("%s: failed to load color 'tasklist_app_active_color'", __func__);
    }

    if (!context->lookup_color("tasklist_app_indicator_color", indicator_color)) {
        g_warning("%s: failed to load color 'tasklist_app_indicator_color'", __func__);
    }

    if (kiran_app_is_active(app_)) {
        Gdk::Cairo::set_source_rgba(cr, active_color);
        cr->paint();

        Gdk::Cairo::set_source_rgba(cr, indicator_color);
        cr->rectangle(0, allocation.get_height() - ACTIVE_INDICATOR_HEIGHT,
                      allocation.get_width(), ACTIVE_INDICATOR_HEIGHT);
        cr->fill();
    } else {
        context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());
        if (app_->get_taskbar_windows().size() != 0)
        {
            Gdk::Cairo::set_source_rgba(cr, indicator_color);
            cr->arc(allocation.get_width() / 2.0,
		     allocation.get_height() - ACTIVE_INDICATOR_HEIGHT / 2.0,
                    ACTIVE_INDICATOR_HEIGHT / 2.0,
                    0,
                    2 * M_PI);
            cr->fill();
        }
    }

    cr->save();
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
        }
    } catch (const Glib::Error &e) {
        std::cerr<<"Error occured while trying to load app icon: "<<e.what()<<std::endl;
        pixbuf.clear();
    }

    if (!pixbuf)
    {
        //未能找到对应应用的图标，使用内置的默认图标
        pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-tasklist/icon/executable",
                                                   icon_size * scale,
                                                   icon_size * scale);
    }

    cr->scale(1.0 / scale, 1.0 / scale);
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                  (allocation.get_width() - icon_size) / 2.0 * scale,
                                  (allocation.get_height() - icon_size - 4) / 2.0 * scale);
    cr->paint();
    cr->restore();


    return false;
}

bool KiranTasklistAppButton::on_button_press_event(GdkEventButton *button_event)
{
    auto app_  = get_app();

    if (!app_) {
        g_warning("%s: app already expired", __FUNCTION__);
        return false;
    }

    if (gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        if (!context_menu) {
            Gtk::MenuItem *item = nullptr;
            context_menu = new KiranAppContextMenu(app_);

        } else {
            //刷新右键菜单内容，因为收藏夹等选项可能需要更新
            g_debug("%s: refresh context menu\n", __FUNCTION__);
            context_menu->refresh();
        }
        context_menu->show_all();

        context_menu->popup_at_pointer((GdkEvent*)button_event);
        gtk_grab_add(GTK_WIDGET(context_menu->gobj()));
        menu_opened = true;
        context_menu->signal_deactivate().connect([this]() -> void {
            this->menu_opened = false;
            gtk_grab_remove(GTK_WIDGET(this->context_menu->gobj()));
        });
        return true;
    }
    set_state_flags(Gtk::STATE_FLAG_ACTIVE, false);

    auto windows_list = app_->get_taskbar_windows();
    if (windows_list.size() == 0) {
        //无已打开窗口，打开新的应用窗口(仅针对常驻任务栏应用)
        app_->launch();
        return true;
    }

    if (windows_list.size() == 1)
    {
        auto first = windows_list.at(0);
        //当前应用只有一个窗口，不需要显示预览窗口，返回true
        g_message("%s: event time %lu\n", __PRETTY_FUNCTION__, button_event->time);
        first->activate(button_event->time);
        return true;
    }

    return false;
}

bool KiranTasklistAppButton::on_button_release_event(GdkEventButton *button_event)
{
    set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_ACTIVE, true);
    return false;
}

bool KiranTasklistAppButton::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
    return false;
}

bool KiranTasklistAppButton::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    set_state_flags(get_state_flags() & ~Gtk::STATE_FLAG_PRELIGHT, true);
    return false;
}


void KiranTasklistAppButton::refresh() {
    Gtk::Allocation allocation;

    allocation = get_allocation();
    queue_draw();
}

const std::shared_ptr<Kiran::App> KiranTasklistAppButton::get_app()
{
    return app.lock();
}

bool KiranTasklistAppButton::get_context_menu_opened()
{
    return menu_opened;
}

Gtk::Orientation KiranTasklistAppButton::get_orientation() const
{
    const Gtk::Box *parent = static_cast<const Gtk::Box*>(get_parent());

    //使用父控件容器的排列方向
    return parent->get_orientation();
}
