#include "kiran-expand-button.h"
#include <glib/gi18n.h>

struct _KiranExpandButton {
    GtkToggleButton parent;
};

typedef struct {
    GtkWidget *label, *image, *box;
}KiranExpandButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(KiranExpandButton, kiran_expand_button, GTK_TYPE_TOGGLE_BUTTON)

void kiran_expand_button_init(KiranExpandButton *self)
{
    KiranExpandButtonPrivate *priv = kiran_expand_button_get_instance_private(self);
    GtkStyleContext *context;

    priv->label = gtk_label_new(_("Expand"));
    priv->image = gtk_image_new_from_resource("/kiran-menu/icon/expand");
    priv->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);

    gtk_label_set_xalign(GTK_LABEL(priv->label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(priv->label), 0.5);
    gtk_container_add(GTK_CONTAINER(priv->box), priv->label);
    gtk_container_add(GTK_CONTAINER(priv->box), priv->image);
    gtk_container_add(GTK_CONTAINER(self), priv->box);

    context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(context, "kiran-expand-button");
}

static void kiran_expand_button_toggled(GtkToggleButton *button)
{
    gboolean active;
    KiranExpandButton *self = KIRAN_EXPAND_BUTTON(button);
    KiranExpandButtonPrivate *priv;

    priv = kiran_expand_button_get_instance_private(self);
    active = gtk_toggle_button_get_active(button);

    gtk_label_set_text(GTK_LABEL(priv->label), active?_("Shrink"):_("Expand"));
    gtk_image_set_from_resource(GTK_IMAGE(priv->image), active?"/kiran-menu/icon/shrink":"/kiran-menu/icon/expand");
}

void kiran_expand_button_class_init(KiranExpandButtonClass *klass)
{
    GTK_TOGGLE_BUTTON_CLASS(klass)->toggled = kiran_expand_button_toggled;
}

GtkWidget *kiran_expand_button_new(gboolean expanded)
{
    return g_object_new(KIRAN_TYPE_EXPAND_BUTTON, "active", expanded, NULL);
}
