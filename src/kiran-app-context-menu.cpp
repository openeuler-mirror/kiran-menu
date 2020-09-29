#include "kiran-app-context-menu.h"
#include <menu-skeleton.h>
#include <glib/gi18n.h>
#include "kiranhelper.h"

KiranAppContextMenu::KiranAppContextMenu(const std::shared_ptr<Kiran::App> &app_)
{
    app = app_;
    get_style_context()->add_class("previewer-context-menu");
    refresh();
}

void KiranAppContextMenu::refresh()
{
    Gtk::MenuItem *item = nullptr;
    std::shared_ptr<Kiran::App> app_ = app.lock();


    //清空之前的菜单项
    KiranHelper::remove_all_for_container(*this);

    if (!app_) {
        g_warning("KiranAppContextMenu: found finalized app\n");
        return;
    }

    for (auto action_name: app_->get_actions()) {
        item = Gtk::manage(new Gtk::MenuItem(app_->get_action_name(action_name)));

        item->signal_activate().connect(
                    [action_name, this]() -> void {
            if (!this->app.expired())
                this->app.lock()->launch_action(action_name);
        });
        append(*item);
    }

    item = Gtk::manage(new Gtk::MenuItem(_("Close all windows")));
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->app.expired())
                        this->app.lock()->close_all_windows();
                });
    if (app_->get_taskbar_windows().size() == 0)
        item->set_sensitive(false);
    append(*item);


    if (!KiranHelper::app_is_in_favorite(app_)) {
        item = Gtk::manage(new Gtk::MenuItem(_("Add to favorites")));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired())
                            KiranHelper::add_app_to_favorite(this->app.lock());
                    });
    } else {
        item = Gtk::manage(new Gtk::MenuItem(_("Remove from favorites")));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired())
                            KiranHelper::remove_app_from_favorite(this->app.lock());
                    });
    }
    append(*item);

    if (!KiranHelper::app_is_in_fixed_list(app_)) {
        item = Gtk::manage(new Gtk::MenuItem(_("Pin to taskbar")));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired())
                            KiranHelper::add_app_to_fixed_list(this->app.lock());
                    });
    } else {
        item = Gtk::manage(new Gtk::MenuItem(_("Unpin to taskbar")));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired())
                            KiranHelper::remove_app_from_fixed_list(this->app.lock());
                    });
    }
    append(*item);
}
