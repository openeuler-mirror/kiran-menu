#ifndef __KIRAN_TRAY_MANAGER__
#define __KIRAN_TRAY_MANAGER__
#include <gtk/gtk.h>
#include "kiran-notify-icon.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_TRAY_MANAGER 		(kiran_tray_manager_get_type ())
#define KIRAN_TRAY_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_TRAY_MANAGER, KiranTrayManager))
#define KIRAN_IS_TRAY_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_TRAY_MANAGER))
#define KIRAN_TRAY_MANAGER_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE ((obj), KIRAN_TYPE_TRAY_MANAGER, KiranTrayManagerInterface))

typedef struct _KiranTrayManager 		KiranTrayManager;
typedef struct _KiranTrayManagerInterface 	KiranTrayManagerInterface;

struct _KiranTrayManagerInterface
{
    GTypeInterface parent;

    void (* set_icon_size)              (KiranTrayManager *manager,
		    			 gint icon_size);
};

GType kiran_tray_manager_get_type (void);
void  kiran_tray_manager_emit_icon_added (KiranTrayManager *manager,
 					  KiranNotifyIcon *icon);
void  kiran_tray_manager_emit_icon_removed (KiranTrayManager *manager,
					    KiranNotifyIcon *icon);
void kiran_tray_manager_set_icon_size (KiranTrayManager *manager,
				       gint icon_size);

G_END_DECLS

#endif /* __KIRAN_TRAY_MANAGER__ */
