#ifndef KIRAN_POWER_BUTTON_INCLUDE_H
#define KIRAN_POWER_BUTTON_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranPowerButton KiranPowerButton;

typedef struct {
    GtkButtonClass parent_class;
} KiranPowerButtonClass;

#define KIRAN_TYPE_POWER_BUTTON     kiran_power_button_get_type()
#define KIRAN_IS_POWER_BUTTON(o)    G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_POWER_BUTTON)
#define KIRAN_POWER_BUTTON(o)       G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_POWER_BUTTON, KiranPowerButton)

GType kiran_power_button_get_type(void);

KiranPowerButton *kiran_power_button_new(void);

#endif