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
    orient(Gtk::ORIENTATION_HORIZONTAL),
    menu_opened(false),
    context_menu(nullptr)
{
    add(drawing_area);

    auto context = get_style_context();
    context->add_class("kiran-tasklist-button");
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    set_has_window(true);

    if (app_->get_taskbar_windows().size() == 0)
        set_tooltip_text(app->get_locale_name());
}

KiranTasklistAppButton::~KiranTasklistAppButton()
{
    if (context_menu)
        delete context_menu;
}

void KiranTasklistAppButton::set_orientation(Gtk::Orientation orient_)
{
    orient = orient_;
    if (orient == Gtk::ORIENTATION_HORIZONTAL) {
        set_hexpand(false);
        set_vexpand(true);
        set_valign(Gtk::ALIGN_FILL);
    } else {
        set_hexpand(true);
        set_vexpand(false);
        set_halign(Gtk::ALIGN_FILL);
    }
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
    if (orient == Gtk::ORIENTATION_HORIZONTAL)
        return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    else
        return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

void KiranTasklistAppButton::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    Gtk::Allocation allocation;

    allocation = get_parent()->get_allocation();
    if (orient == Gtk::ORIENTATION_HORIZONTAL) {
        minimum_width = natural_width = allocation.get_height();
    } else
        minimum_width = natural_width = allocation.get_width();
}

void KiranTasklistAppButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    Gtk::Allocation allocation;

    allocation = get_parent()->get_allocation();
    if (orient == Gtk::ORIENTATION_VERTICAL) {
        minimum_height = natural_height = allocation.get_width();
    } else
        minimum_height = natural_height = allocation.get_height();
}

void KiranTasklistAppButton::get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const
{
    minimum_width = natural_width = height;
}

void KiranTasklistAppButton::get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const
{
    minimum_height = natural_height = width;
}

void KiranTasklistAppButton::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::EventBox::on_size_allocate(allocation);

    if (orient == Gtk::ORIENTATION_VERTICAL) {
        icon_size = allocation.get_width() - 8;

    } else {
        icon_size = allocation.get_height() - 8;
    }
}

bool KiranTasklistAppButton::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    auto context = get_style_context();
    auto gicon = app->get_icon();
    auto icon_theme = Gtk::IconTheme::get_default();


    allocation = get_allocation();
    context->set_state(get_state_flags());

    if (kiran_app_is_active(app)) {
        cr->set_source_rgba(1.0, 1.0, 1.0, 0.3);
        cr->paint();
    } else {
        context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());
    }

    cr->save();
    try {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	int scale = get_scale_factor();
        if (gicon)
        {
            //将应用图标转换为pixbuf
            std::string icon_desc = gicon->to_string();

            if (icon_desc[0] == '/') {
                pixbuf = Gdk::Pixbuf::create_from_file(icon_desc, icon_size * scale, icon_size * scale);
	    }
            else
            {

                pixbuf = icon_theme->load_icon(gicon->to_string(),
                                               icon_size,
                                               scale,
                                               Gtk::ICON_LOOKUP_FORCE_SIZE);
            }
        } else {
            //未能找到对应应用的图标，使用内置的默认图标
            pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-tasklist/icon/executable",
                                                        icon_size * scale,
                                                        icon_size * scale);
        }

	cr->scale(1.0/scale, 1.0/scale);
        Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                      (allocation.get_width() - icon_size)/2.0 * scale,
                                      (allocation.get_height() - icon_size - 4)/2.0 * scale);
        cr->paint();
        cr->restore();

        if (kiran_app_is_active(app)) {
            Gdk::RGBA color("#3ca8ea");

            Gdk::Cairo::set_source_rgba(cr, color);
            cr->rectangle(0, allocation.get_height() - ACTIVE_INDICATOR_HEIGHT,
                          allocation.get_width(), ACTIVE_INDICATOR_HEIGHT);
            cr->fill();
        }
        else if (app->get_taskbar_windows().size() != 0) {
            Gdk::RGBA color("#3ca8ea");

            Gdk::Cairo::set_source_rgba(cr, color);

            cr->arc(allocation.get_width()/2.0, allocation.get_height() - ACTIVE_INDICATOR_HEIGHT/2.0,
                    ACTIVE_INDICATOR_HEIGHT/2.0, 0, 2 * M_PI);
            cr->fill();
        }
    } catch (const Glib::Error &e) {
        std::cerr<<"Error occured while trying to load app icon: "<<e.what()<<std::endl;
    }

    return false;
}

void KiranTasklistAppButton::on_realize()
{
    Gtk::Allocation allocation;
    GdkWindowAttr attrs;
    int mask = GDK_WA_X | GDK_WA_Y;
    set_realized(true);

    allocation = get_allocation();

    memset(&attrs, 0, sizeof(attrs));
    attrs.width = allocation.get_width();
    attrs.height = allocation.get_height();
    attrs.x = allocation.get_x();
    attrs.y = allocation.get_y();
    attrs.window_type = GDK_WINDOW_CHILD;
    attrs.event_mask = GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
            GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
    attrs.wclass = GDK_INPUT_OUTPUT;

    window = Gdk::Window::create(this->get_parent_window(), &attrs, mask);

    set_window(window);
    register_window(window);
}

bool KiranTasklistAppButton::on_button_press_event(GdkEventButton *button_event)
{
    auto windows_list = app->get_taskbar_windows();

    if (gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        if (!context_menu) {
            Gtk::MenuItem *item = nullptr;
            context_menu = new KiranAppContextMenu(app);

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

    if (windows_list.size() == 0) {
        //无已打开窗口，打开新的应用窗口(仅针对常驻任务栏应用)
        app->launch();
        return false;
    }

    //遍历窗口列表
    std::vector<KiranWindowPointer>::iterator iter =
             std::upper_bound(windows_list.begin(), windows_list.end(), last_raised.lock(),
                     [](KiranWindowPointer w1, KiranWindowPointer w2) -> bool {
                        if (!w1)
                            return true;
                        if (!w2)
                            return false;

                        return w1->get_xid() > w2->get_xid();
                     });

    if (iter != windows_list.end()) {
        //设置新的活动窗口
        last_raised = *iter;
    }
    else {
        //没有ID比当前窗口还大的窗口，默认使用该应用的第一个窗口
        auto first = windows_list.at(0);
        if (last_raised.lock() == first) {
            //当前应用只有一个窗口，而且是当前窗口，那么将当前窗口最小化

            first->activate(button_event->time);
            return false;
        } else
            last_raised = first;
    }

    last_raised.lock()->activate(button_event->time);

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
    return app;
}

sigc::signal<void,Gtk::Orientation> KiranTasklistAppButton::signal_orient_changed()
{
    return m_signal_orient_changed;
}
bool KiranTasklistAppButton::get_context_menu_opened()
{
    return menu_opened;
}
