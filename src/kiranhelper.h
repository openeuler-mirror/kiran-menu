#ifndef KIRANHELPER_H
#define KIRANHELPER_H

#include <gtkmm.h>

class KiranHelper
{
public:
    KiranHelper();

    static void remove_widget(Gtk::Widget &widget);
    static void remove_all_for_container(Gtk::Container &container);
    static bool grab_input(Gtk::Widget &widget);
    static void ungrab_input(Gtk::Widget &widget);
};

#endif // KIRANHELPER_H
