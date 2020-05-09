#ifndef KIRAN_MENU_WINDOW_INCLUDE_H
#define KIRAN_MENU_WINDOW_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranMenuWindow KiranMenuWindow;
typedef struct {
    GObjectClass parent_class;
} KiranMenuWindowClass;

#define KIRAN_TYPE_MENU_WINDOW  kiran_menu_window_get_type()
#define KIRAN_IS_MENU_WINDOW(o) G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_MENU_WINDOW)
#define KIRAN_MENU_WINDOW(o)    G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_MENU_WINDOW, KiranMenuWindow)

GType kiran_menu_window_get_type();

KiranMenuWindow *kiran_menu_window_new(GtkWidget *parent);
GtkWidget *kiran_menu_window_get_window(KiranMenuWindow *window);
void kiran_menu_window_reset_layout(KiranMenuWindow *window);

#endif
