#ifndef KIRAN_EXPAND_BUTTON_INCLUDE_H
#define KIRAN_EXPAND_BUTTON_INCLUDE_H

#include <gtk/gtk.h>

#define KIRAN_TYPE_EXPAND_BUTTON        kiran_expand_button_get_type()

G_DECLARE_FINAL_TYPE(KiranExpandButton, kiran_expand_button, KIRAN, EXPAND_BUTTON, GtkToggleButton)

GtkWidget *kiran_expand_button_new(gboolean expanded);

#endif      //KIRAN_EXPAND_BUTTON_INCLUDE_H