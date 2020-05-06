#ifndef KIRAN_APP_BUTTON_INCLUDE_H
#define KIRAN_APP_BUTTON_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranAppButton KiranAppButton;

typedef struct {
    GtkButtonClass parent_class;
} KiranAppButtonClass;

#define KIRAN_TYPE_APP_BUTTON       kiran_app_button_get_type()
#define KIRAN_IS_APP_BUTTON(o)      G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_APP_BUTTON)
#define KIRAN_APP_BUTTON(o)         G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_APP_BUTTON, KiranAppButton)

GType kiran_app_button_get_type();

KiranAppButton *kiran_app_button_new(const char *icon_file,
        const char *tooltip,
        const char *exec_command);

#endif