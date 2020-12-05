#include "menu-power-button.h"

#include <glibmm/i18n.h>

MenuPowerButton::MenuPowerButton():
    menu(nullptr)
{
    auto context = get_style_context();

    icon.set_from_resource("/kiran-menu/sidebar/power");
    add(icon);
    set_tooltip_text(_("Power options"));
    context->add_class("menu-app-launcher");
}

MenuPowerButton::~MenuPowerButton()
{
    delete menu;
}

void MenuPowerButton::on_clicked()
{
    GdkEvent *event = gtk_get_current_event();

    if (menu)
        delete menu;

    menu = new MenuPowerMenu();
    menu->attach_to_widget(*this);
    menu->show_all();
    menu->popup_at_widget(this, Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_SOUTH_WEST, event);

    gdk_event_free(event);
}
