#include "kiran-menu-power-menu.h"
#include "kiranpower.h"
#include <glibmm/i18n.h>
#include <iostream>
#include "kiranhelper.h"

KiranMenuPowerMenu::KiranMenuPowerMenu()
{
    logout_item.set_label(_("Logout"));
    suspend_item.set_label(_("Suspend"));
    hibernate_item.set_label(_("Hibernate"));
    shutdown_item.set_label(_("Shutdown"));
    reboot_item.set_label(_("Reboot"));

    append(logout_item);
    if (KiranPower::can_suspend())
        append(suspend_item);

    if (KiranPower::can_hibernate())
        append(hibernate_item);
    append(reboot_item);
    append(shutdown_item);



    logout_item.signal_activate().connect(sigc::hide_return(sigc::ptr_fun(&KiranPower::logout)));
    suspend_item.signal_activate().connect(sigc::hide_return(sigc::ptr_fun(&KiranPower::suspend)));
    hibernate_item.signal_activate().connect(sigc::hide_return(sigc::ptr_fun(&KiranPower::hibernate)));
    reboot_item.signal_activate().connect(sigc::hide_return(sigc::ptr_fun(&KiranPower::reboot)));
    shutdown_item.signal_activate().connect(sigc::hide_return(sigc::ptr_fun(&KiranPower::shutdown)));


    logout_item.signal_activate().connect(sigc::mem_fun(*this, &KiranMenuPowerMenu::hide_menu_window));
    suspend_item.signal_activate().connect(sigc::mem_fun(*this, &KiranMenuPowerMenu::hide_menu_window));
    hibernate_item.signal_activate().connect(sigc::mem_fun(*this, &KiranMenuPowerMenu::hide_menu_window));
    reboot_item.signal_activate().connect(sigc::mem_fun(*this, &KiranMenuPowerMenu::hide_menu_window));
    shutdown_item.signal_activate().connect(sigc::mem_fun(*this, &KiranMenuPowerMenu::hide_menu_window));

}

KiranMenuPowerMenu::~KiranMenuPowerMenu()
{

}

void KiranMenuPowerMenu::hide_menu_window()
{
    Gtk::Widget *menu_window;

    if (!this->get_attach_widget())
        return;
    menu_window = this->get_attach_widget()->get_toplevel();
    menu_window->hide();
}

bool KiranMenuPowerMenu::on_map_event(GdkEventAny *any_event)
{
    bool res;

    res = Gtk::Menu::on_map_event(any_event);

        select_item(logout_item);;
    return res;
}

bool KiranMenuPowerMenu::on_focus_in_event(GdkEventFocus *focus_event)
{
    bool res;

    res = Gtk::Menu::on_focus_in_event(focus_event);
        logout_item.grab_focus();
    std::cout<<"focus in"<<std::endl;
    return res;
}
