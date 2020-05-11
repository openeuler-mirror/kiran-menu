#ifndef KIRAN_POWER_MENU_INCLUDE_H
#define KIRAN_POWER_MENU_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranPowerMenu KiranPowerMenu;

typedef struct _KiranPowerMenuClass KiranPowerMenuClass;

#define KIRAN_TYPE_POWER_MENU       kiran_power_menu_get_type()
#define KIRAN_IS_POWER_MENU(o)      G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_POWER_MENU)
#define KIRAN_POWER_MENU(o)         G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_POWER_MENU, KiranPowerMenu)

GType kiran_power_menu_get_type();

KiranPowerMenu *kiran_power_menu_new(void);

#endif