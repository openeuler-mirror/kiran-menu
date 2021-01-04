#ifndef __KIRAN_SN_ICON_H__ 
#define __KIRAN_SN_ICON_H__
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_SN_ICON		(kiran_sn_icon_get_type ())
#define KIRAN_SN_ICON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_SN_ICON, KiranSnIcon))
#define KIRAN_SN_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((obj), KIRAN_TYPE_SN_ICON, KiranSnIconClass))
#define KIRAN_IS_SN_ICON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_SN_ICON))
#define KIRAN_IS_SN_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KIRAN_TYPE_SN_ICON))
#define KIRAN_SN_ICON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), KIRAN_TYPE_SN_ICON, KiranSnIconClass))

typedef struct _KiranSnIcon 		KiranSnIcon;
typedef struct _KiranSnIconClass 	KiranSnIconClass;
typedef struct _KiranSnIconPrivate 	KiranSnIconPrivate;

struct _KiranSnIcon
{
    GtkButton parent_class;

    KiranSnIconPrivate *priv;
};

struct _KiranSnIconClass
{
    GtkButtonClass parent_class;
};

GType kiran_sn_icon_get_type (void);

GtkWidget *kiran_sn_icon_new (const gchar *bus_name,
		              const gchar *object_path,
			      gint        icon_size);
const gchar *kiran_sn_icon_get_bus_name (KiranSnIcon *icon);
const gchar *kiran_sn_icon_get_object_path (KiranSnIcon *icon);
void  kiran_sn_icon_set_icon_size (KiranSnIcon *icon,
				   gint icon_size);

G_END_DECLS

#endif /*__KIRAN_SN_ICON_H__ */
