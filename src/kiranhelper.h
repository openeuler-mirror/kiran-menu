#ifndef KIRANHELPER_H
#define KIRANHELPER_H

#include <gtkmm.h>
#include <app-manager.h>

using KiranAppPointer = std::shared_ptr<Kiran::App>;
using KiranWindowPointer = std::shared_ptr<Kiran::Window>;

class KiranHelper
{
public:
    KiranHelper() = delete;
    ~KiranHelper() = delete;

    static void remove_widget(Gtk::Widget &widget);
    static void remove_all_for_container(Gtk::Container &container, bool need_free = true);
    static bool grab_input(Gtk::Widget &widget);
    static void ungrab_input(Gtk::Widget &widget);

    static bool window_is_ignored(KiranWindowPointer window);

    static bool app_is_in_favorite(const std::shared_ptr<Kiran::App> &app);
    static bool add_app_to_favorite(const std::shared_ptr<Kiran::App> &app);
    static bool remove_app_from_favorite(const std::shared_ptr<Kiran::App> &app);

    static bool app_is_in_fixed_list(const std::shared_ptr<Kiran::App> &app);
    static bool add_app_to_fixed_list(const std::shared_ptr<Kiran::App> &app);
    static bool remove_app_from_fixed_list(const std::shared_ptr<Kiran::App> &app);
};

#endif // KIRANHELPER_H
