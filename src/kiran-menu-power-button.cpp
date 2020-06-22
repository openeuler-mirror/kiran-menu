#include "kiran-menu-power-button.h"

#include <glibmm/i18n.h>

KiranMenuPowerButton::KiranMenuPowerButton()
{
    auto context = get_style_context();

    menu = new KiranMenuPowerMenu();
    menu->attach_to_widget(*this);
    menu->show_all();

    icon.set_from_resource("/kiran-menu/sidebar/power");
    add(icon);
    set_tooltip_text(_("Power options"));
    context->add_class("kiran-app-button");
}

KiranMenuPowerButton::~KiranMenuPowerButton()
{
    delete menu;
}

void KiranMenuPowerButton::on_clicked()
{
    GdkEvent *event = gtk_get_current_event();

    menu->popup_at_widget(this, Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_SOUTH_WEST, event);

    gdk_event_free(event);
}
