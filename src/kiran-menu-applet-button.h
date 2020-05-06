#ifndef KIRAN_MENU_APPLET_BUTTON_INCLUDE_H
#define KIRAN_MENU_APPLET_BUTTON_INCLUDE_H

#include <mate-panel-applet.h>
#include <gtk/gtk.h>

typedef struct _KiranMenuAppletButton KiranMenuAppletButton;
typedef struct {
    GtkToggleButtonClass parent_class;
} KiranMenuAppletButtonClass;

#define KIRAN_TYPE_MENU_APPLET_BUTTON   kiran_menu_applet_button_get_type()
#define KIRAN_IS_MENU_APPLET_BUTTON(o)  G_TYPE_CHECK_INSTANCE_TYPE(o, kiran_menu_applet_button_get_type())
#define KIRAN_MENU_APPLET_BUTTON(o)     G_TYPE_CHECK_INSTANCE_CAST(o, kiran_menu_applet_button_get_type(), KiranMenuAppletButton)

GType kiran_menu_applet_button_get_type();

KiranMenuAppletButton *kiran_menu_applet_button_new(MatePanelApplet *applet);

#endif //KIRAN_MENU_APPLET_INCLUDE_H