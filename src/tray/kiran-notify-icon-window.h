#ifndef __KIRAN_NOTIFY_ICON_WINDOW_H__
#define __KIRAN_NOTIFY_ICON_WINDOW_H__

#include <gtk/gtk.h>
#include "kiran-notify-icon.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_NOTIFY_ICON_WINDOW  			(kiran_notify_icon_window_get_type ())
#define KIRAN_NOTIFY_ICON_WINDOW(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_NOTIFY_ICON_WINDOW, KiranNotifyIconWindow))
#define KIRAN_NOTIFY_ICON_WINDOW_CALSS(klass)        	(G_TYPE_CHECK_CLASS_CAST ((klass), KIRAN_TYPE_NOTIFY_ICON_WINDOW, KiranNotifyIconWindowClass))
#define KIRAN_IS_NOTIFY_ICON_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_NOTIFY_ICON_WINDOW))
#define KIRAN_IS_NOTIFY_ICON_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KIRAN_TYPE_NOTIFY_ICON_WINDOW))
#define KIRAN_NOTIFY_ICON_GET_CLASS(obj)		(G_TYPE_GET_INSTANCE_CLASS ((obj), KiranNotifyIconWindowClass))

typedef struct _KiranNotifyIconWindow 		KiranNotifyIconWindow;
typedef struct _KiranNotifyIconWindowClass 	KiranNotifyIconWindowClass;
typedef struct _KiranNotifyIconWindowPrivate  	KiranNotifyIconWindowPrivate;

struct _KiranNotifyIconWindow
{
    GtkWindow parent;

    KiranNotifyIconWindowPrivate *priv;
};

struct _KiranNotifyIconWindowClass
{
    GtkWindowClass parent_class;
};

GType kiran_notify_icon_window_get_type (void);

GtkWidget *kiran_notify_icon_window_new (void);
void       kiran_notify_icon_window_add_icon (KiranNotifyIconWindow *window,
					      KiranNotifyIcon *icon);
void       kiran_notify_icon_window_remove_icon (KiranNotifyIconWindow *window,
					         KiranNotifyIcon *icon);
gint       kiran_notify_icon_window_get_icons_number (KiranNotifyIconWindow *window);

G_END_DECLS

#endif /* __KIRAN_NOTIFY_ICON_WINDOW_H__ */
