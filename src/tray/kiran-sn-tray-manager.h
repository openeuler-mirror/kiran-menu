#ifndef __KIRAN_SN_TRAY_MANAGER_H__
#define __KIRAN_SN_TRAY_MANAGER_H__

#include "kiran-sn-host-gen.h"
#include "kiran-tray-manager.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_SN_TRAY_MANAGER 			(kiran_sn_tray_manager_get_type ())
#define KIRAN_SN_TRAY_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_SN_TRAY_MANAGER, KiranSnTrayManager))
#define KIRAN_SN_TRAY_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST ((klass), KIRAN_TYPE_SN_TRAY_MANAGER, KiranSnTrayManagerClass))
#define KIRAN_IS_SN_TRAY_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_SN_TRAY_MANAGER))
#define KIRAN_IS_SN_TRAY_MANAGET_CLAS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), KIRAN_TYPE_SN_TRAY_MANAGER))
#define KIRAN_SN_TRAY_MANAGER_GET_CLASS(obj)		(G_TYPE_CHECK_INSTANCE_GET_CLASS ((obj), KIRAN_TYPE_SN_TRAY_MANAGER, KiranSnTrayManagerClass))

typedef struct _KiranSnTrayManager		KiranSnTrayManager;
typedef struct _KiranSnTrayManagerClass		KiranSnTrayManagerClass;
typedef struct _KiranSnTrayManagerPrivate	KiranSnTrayManagerPrivate;


struct _KiranSnTrayManager
{
    KiranSnHostGenSkeleton parent;

    KiranSnTrayManagerPrivate *priv;
};

struct _KiranSnTrayManagerClass
{
    KiranSnHostGenSkeletonClass parent_class;
};

GType 		 kiran_sn_tray_manager_get_type (void);
KiranTrayManager *kiran_sn_tray_manager_new (void);

G_END_DECLS

#endif /* __KIRAN_SN_TRAY_MANAGER_H__ */
