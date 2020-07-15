#include "kiran-window-context-menu.h"
#include "kiranhelper.h"
#include <glib/gi18n.h>
#include <gtk/gtkx.h>


KiranWindowContextMenu::KiranWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_):
    win(win_)
{
    refresh();
    get_style_context()->add_class("previewer-context-menu");
}

void KiranWindowContextMenu::refresh()
{
    Gtk::MenuItem *item = nullptr;
    Gtk::CheckMenuItem *check_item;

    KiranHelper::remove_all_for_container(*this);

    item = Gtk::manage(new Gtk::MenuItem(_("Maximize")));
    /*
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired()) {

                        auto window = this->win.lock();
                        if (window->is_minimized()) {
                            //如果窗口已经最小化，需要先恢复窗口大小
                            window->unminimize(gtk_get_current_event_time());
                        }
                        window->maximize();
                    }
                });
		*/
    append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("Minimize")));
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired())
                        this->win.lock()->minimize();
                });
    append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("Restore")));
    /*
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired()) {
                        std::shared_ptr<Kiran::Window> win;
                        win = this->win.lock();
                        GdkEvent *event = gtk_get_current_event();

                        if (win->is_minimized())
                            win->unminimize(gdk_event_get_time(event));

                        if (win->is_maximized())
                            win->unmaximize();
                    }

                });
		*/
    append(*item);

    check_item = Gtk::manage(new Gtk::CheckMenuItem(_("Always keep on top")));
    check_item->signal_activate().connect(
                [this, check_item]() -> void {
                    if (!this->win.expired()) {
                        auto window = this->win.lock();
                        if (check_item->get_active())
                            window->make_above();
                        else {
                           // window->unmake_above();
                        }
                    }

                });
    check_item->set_active(this->win.lock()->is_above());
    append(*check_item);


    item = Gtk::manage(new Gtk::MenuItem(_("Close Window")));
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired())
                        this->win.lock()->close();
                });
    append(*item);

    show_all();
}
