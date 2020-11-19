#include "kiran-helper.h"
#include "menu-skeleton.h"
#include "taskbar-skeleton.h"

void KiranHelper::remove_widget(Gtk::Widget &widget)
{
    if (widget.get_parent())
        widget.get_parent()->remove(widget);
}

void KiranHelper::remove_all_for_container(Gtk::Container &container, bool need_free)
{
    for (auto widget: container.get_children()) {
        container.remove(*widget);

        if (need_free)
            delete widget;
    }
}

bool KiranHelper::grab_input(Gtk::Widget &widget)
{
    Gdk::GrabStatus status;
    auto display = Gdk::Display::get_default();
    auto seat = display->get_default_seat();
    status = seat->grab(widget.get_window(),
                    Gdk::SEAT_CAPABILITY_ALL_POINTING | Gdk::SEAT_CAPABILITY_KEYBOARD,
                    true);
    return status == Gdk::GRAB_SUCCESS;
}

void KiranHelper::ungrab_input(Gtk::Widget &widget)
{
    auto display = Gdk::Display::get_default();
    auto seat = display->get_default_seat();

    seat->ungrab();
}

bool KiranHelper::window_is_ignored(KiranWindowPointer window)
{
    if (!window)
        return true;

    return window->get_window_type() != WNCK_WINDOW_NORMAL &&
            window->get_window_type() != WNCK_WINDOW_DIALOG;
}

bool KiranHelper::app_is_in_favorite(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::MenuSkeleton::get_instance();

    if (!app)
        return false;

    if (backend->lookup_favorite_app(app->get_desktop_id()) == nullptr)
        return false;
    return true;
}

bool KiranHelper::add_app_to_favorite(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::MenuSkeleton::get_instance();
    if (!app)
        return true;

    return backend->add_favorite_app(app->get_desktop_id());
}

bool KiranHelper::remove_app_from_favorite(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::MenuSkeleton::get_instance();

    if (!app)
        return true;

    return backend->del_favorite_app(app->get_desktop_id());
}

bool KiranHelper::app_is_in_fixed_list(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    return backend->lookup_fixed_app(app->get_desktop_id()) != nullptr;
}

bool KiranHelper::add_app_to_fixed_list(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    g_debug("%s: add app into fixed list\n", __func__);
    return backend->add_fixed_app(app->get_desktop_id());
}

bool KiranHelper::remove_app_from_fixed_list(const std::shared_ptr<Kiran::App> &app)
{
    g_debug("%s: remove app from fixed list\n", __func__);

    auto backend = Kiran::TaskBarSkeleton::get_instance();

    return backend->del_fixed_app(app->get_desktop_id());
}

bool KiranHelper::gdk_rectangle_contains_point(GdkRectangle *rect, GdkPoint *point)
{
    return (point->x >= rect->x && point->x <= rect->x + rect->width) &&
            (point->y >= rect->y && point->y <= rect->y + rect->height);
}

void KiranHelper::cairo_draw_rounded_rect(const Cairo::RefPtr<Cairo::Context> &cr, Gdk::Rectangle &rect, double radius)
{
    cr->begin_new_sub_path();
    cr->arc(rect.get_x() + radius, rect.get_y() + radius, radius, M_PI, 3 * M_PI/2);
    cr->arc(rect.get_x() + rect.get_width() - radius,
            rect.get_y() + radius,
            radius,
            -M_PI/2,
            0);

    cr->arc(rect.get_x() + rect.get_width() - radius,
            rect.get_y() + rect.get_height() - radius,
            radius,
            0,
            M_PI/2);
    cr->arc(rect.get_x() + radius,
            rect.get_y() + rect.get_height() - radius,
            radius,
            M_PI/2,
            M_PI);

    cr->line_to(rect.get_x(), rect.get_y() + radius);
    cr->stroke();
}


void KiranHelper::geometry_to_rect(const Kiran::WindowGeometry &geometry, Gdk::Rectangle &rect)
{
    rect.set_x(std::get<0>(geometry));
    rect.set_y(std::get<1>(geometry));
    rect.set_width(std::get<2>(geometry));
    rect.set_height(std::get<3>(geometry));
}

void KiranHelper::run_commandline(const char *cmdline)
{
    std::vector<std::string> args;
    Glib::SpawnFlags flags;
    gchar **tokens = nullptr;

    flags = Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_CLOEXEC_PIPES;
    tokens = g_strsplit(cmdline, " ", -1);
    for (int i = 0; tokens[i] != nullptr; i++) {
        args.push_back(tokens[i]);
    }

    g_strfreev(tokens);
    Glib::spawn_async(std::string(), args, flags);
}
