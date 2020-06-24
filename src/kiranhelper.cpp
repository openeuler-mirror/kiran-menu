#include "kiranhelper.h"

KiranHelper::KiranHelper()
{

}

void KiranHelper::remove_widget(Gtk::Widget &widget)
{
    if (widget.get_parent())
        widget.get_parent()->remove(widget);
}

void KiranHelper::remove_all_for_container(Gtk::Container &container)
{
    container.foreach(sigc::ptr_fun(&KiranHelper::remove_widget));
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
