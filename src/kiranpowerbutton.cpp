#include "kiranpowerbutton.h"
#include "kiranpowermenu.h"

#include <glibmm/i18n.h>

KiranPowerButton::KiranPowerButton()
{
    auto context = get_style_context();

    menu = new KiranPowerMenu();
    menu->attach_to_widget(*this);
    menu->show_all();

    icon.set_from_resource("/kiran-menu/sidebar/power");
    add(icon);
    set_tooltip_text(_("Power options"));
    context->add_class("kiran-app-button");
}

KiranPowerButton::~KiranPowerButton()
{
    delete menu;
}

void KiranPowerButton::on_clicked()
{
    GdkEvent *event = gtk_get_current_event();

    menu->popup_at_widget(this, Gdk::GRAVITY_SOUTH_EAST, Gdk::GRAVITY_SOUTH_WEST, event);

    gdk_event_free(event);
}
