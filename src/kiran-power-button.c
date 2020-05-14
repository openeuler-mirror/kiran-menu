#include "kiran-power-button.h"
#include "kiran-power-menu.h"

struct _KiranPowerButton {
    GtkButton parent;

    KiranPowerMenu *power_menu;
    GtkWidget *icon;
};

G_DEFINE_TYPE(KiranPowerButton, kiran_power_button, GTK_TYPE_BUTTON)

void kiran_power_button_init(KiranPowerButton *self)
{
    GtkStyleContext *context;

    self->power_menu = kiran_power_menu_new();
    self->icon = gtk_image_new_from_resource("/kiran-menu/sidebar/power");

    gtk_popover_set_relative_to(GTK_POPOVER(self->power_menu), GTK_WIDGET(self));
    gtk_container_add(GTK_CONTAINER(self), self->icon);

    context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(context, "kiran-app-button");
}

void kiran_power_button_finalize(GObject*obj)
{
    KiranPowerButton *self = KIRAN_POWER_BUTTON(obj);

    g_object_unref(self->power_menu);
    G_OBJECT_CLASS(obj)->finalize(obj);
}

void kiran_power_button_clicked(GtkButton *button)
{
    KiranPowerButton *self = KIRAN_POWER_BUTTON(button);

    gtk_popover_popup(GTK_POPOVER(self->power_menu));
}

void kiran_power_button_class_init(KiranPowerButtonClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_power_button_finalize;
    GTK_BUTTON_CLASS(kclass)->clicked = kiran_power_button_clicked;
}

KiranPowerButton *kiran_power_button_new()
{
    return g_object_new(KIRAN_TYPE_POWER_BUTTON, NULL);
}

