#include "kiran-window-context-menu.h"
#include "kiranhelper.h"
#include <glib/gi18n.h>
#include <gtk/gtkx.h>
#include "workspace-manager.h"


KiranWindowContextMenu::KiranWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_):
    win(win_)
{
    refresh();
    get_style_context()->add_class("previewer-context-menu");
}

void KiranWindowContextMenu::refresh()
{
    Gtk::MenuItem *item = nullptr;
    Gtk::RadioMenuItem *radio_item;
    Gtk::CheckMenuItem *check_item;

    KiranHelper::remove_all_for_container(*this);
    auto window = this->win.lock();

    if (!window)
        return;

    item = Gtk::manage(new Gtk::MenuItem(_("Maximize")));
    
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
    append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("Minimize")));
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired())
                        this->win.lock()->minimize();
                });
    append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("Restore")));
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
    append(*item);

    check_item = Gtk::manage(new Gtk::CheckMenuItem(_("Always on top")));
    check_item->signal_activate().connect(
                [this, check_item]() -> void {
                    if (!this->win.expired()) {
                        auto window = this->win.lock();
                        if (check_item->get_active())
                            window->make_above();
                        else {
                            window->make_unabove();
                        }
                    }

                });
    check_item->set_active(this->win.lock()->is_above());
    append(*check_item);

    radio_item = Gtk::manage(new Gtk::RadioMenuItem(group, _("Only on this workspace")));
    radio_item->signal_toggled().connect(
                [this, radio_item]() -> void {
                    if (!this->win.expired() && radio_item->get_active()) {
                        this->win.lock()->set_on_visible_workspace(false);
                    }
                });
    append(*radio_item);
    radio_item->set_active(true);

    radio_item = Gtk::manage(new Gtk::RadioMenuItem(group, _("Always on visible workspace")));
    radio_item->signal_toggled().connect(
                [this, radio_item]() -> void {
                    if (!this->win.expired() && radio_item->get_active()) {
                        this->win.lock()->set_on_visible_workspace(true);
                    }
                });
    append(*radio_item);
    if (window->get_on_visible_workspace())
        radio_item->set_active(true);

    item = Gtk::manage(new Gtk::MenuItem(_("Move to another workspace")));
    do {
        //创建工作区列表子菜单
        auto submenu = create_workspace_menu();

        g_assert(submenu != nullptr);
        item->set_submenu(*submenu);
        submenu->show_all();
    } while (0);
    append(*item);

    item = Gtk::manage(new Gtk::MenuItem(_("Close Window")));
    item->signal_activate().connect(
                [this]() -> void {
                    if (!this->win.expired())
                        this->win.lock()->close();
                });
    append(*item);

    show_all();
}

/**
 * 创建工作区列表菜单
 */
Gtk::Menu *KiranWindowContextMenu::create_workspace_menu(void)
{
    Gtk::MenuItem *item;
    KiranOpacityMenu *menu = Gtk::manage(new KiranOpacityMenu());
    Kiran::WorkspaceManager *manager = Kiran::WorkspaceManager::get_instance();

    auto window = this->win.lock();

    if (!window)
        return nullptr;

    for (auto workspace: manager->get_workspaces()) {
        item = Gtk::manage(new Gtk::MenuItem(workspace->get_name()));
        item->signal_activate().connect(
                    [workspace, this]() -> void {
            /**
             * 将窗口移动到对应的工作区
             */
            auto window = this->win.lock();
            if (window)
                window->move_to_workspace(workspace);
        });

        menu->append(*item);
        if (workspace == window->get_workspace())
            item->set_sensitive(false);
    }

    menu->get_style_context()->add_class("previewer-context-menu");

    return menu;
}
