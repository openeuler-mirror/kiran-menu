#include "kiran-helper.h"
#include "menu-skeleton.h"
#include "taskbar-skeleton.h"
#include "workspace-manager.h"
#include "log.h"

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

    LOG_DEBUG("add app into fixed list");
    return backend->add_fixed_app(app->get_desktop_id());
}

bool KiranHelper::remove_app_from_fixed_list(const std::shared_ptr<Kiran::App> &app)
{
    LOG_DEBUG("remove app from fixed list");

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

bool KiranHelper::window_is_on_active_workspace(const std::shared_ptr<Kiran::Window> &window)
{
    auto active_workspace = Kiran::WorkspaceManager::get_instance()->get_active_workspace();

    g_return_val_if_fail(window != nullptr, false);
    g_return_val_if_fail(active_workspace != nullptr, false);

    return window->get_workspace() == active_workspace;
}

Kiran::WindowVec KiranHelper::get_active_workspace_windows(const std::shared_ptr<Kiran::App> &app)
{
    g_return_val_if_fail(app != nullptr, Kiran::WindowVec());
    auto windows = app->get_taskbar_windows();

    auto removed = std::remove_if(windows.begin(), windows.end(),
                                  [](const std::shared_ptr<Kiran::Window> window) -> bool {
                                      return !window_is_on_active_workspace(window);
                                  });

    windows.erase(removed, windows.end());
    return windows;
}

Kiran::WindowVec KiranHelper::get_taskbar_windows(const std::shared_ptr<Kiran::App> &app)
{
    auto backend = Kiran::TaskBarSkeleton::get_instance();
    if (backend->get_app_show_policy() == Kiran::TaskBarSkeleton::POLICY_SHOW_ACTIVE_WORKSPACE)
        return KiranHelper::get_active_workspace_windows(app);
    else
        return app->get_taskbar_windows();
}


bool KiranHelper::launch_app_from_list(const char **app_names)
{
    std::shared_ptr<Kiran::App> app;
    auto app_manager = Kiran::AppManager::get_instance();

    for (int i = 0; app_names[i] != nullptr; i++) {
        app = app_manager->lookup_app(std::string(app_names[i]) + ".desktop");
        if (app)
            break;
    }

    if (app) {
        app->launch();
        return true;
    }

    return false;
}