#include "kiranhelper.h"
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
                    Gdk::SEAT_CAPABILITY_ALL_POINTING,
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
