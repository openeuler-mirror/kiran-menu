#ifndef KIRANHELPER_H
#define KIRANHELPER_H

#include <gtkmm.h>
#include <app-manager.h>

using KiranAppPointer = std::shared_ptr<Kiran::App>;
using KiranWindowPointer = std::shared_ptr<Kiran::Window>;

class KiranHelper
{
public:
    KiranHelper();

    static void remove_widget(Gtk::Widget &widget);
    static void remove_all_for_container(Gtk::Container &container);
    static bool grab_input(Gtk::Widget &widget);
    static void ungrab_input(Gtk::Widget &widget);

    static bool window_is_ignored(KiranWindowPointer window);
};

#endif // KIRANHELPER_H
