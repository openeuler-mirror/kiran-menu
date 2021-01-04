#ifndef __KIRAN_SN_WATCHER__
#define __KIRAN_SN_WATCHER__

#include "kiran-sn-watcher-gen.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_SN_WATCHER		(kiran_sn_watcher_get_type ())
#define KIRAN_SN_WATCHER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_SN_WATCHER, KiranSnWatcher))
#define KIRAN_IS_SN_WATCHER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), KIRAN_TYPE_SN_WATCHER))

typedef struct _KiranSnWatcher KiranSnWatcher;
typedef struct _KiranSnWatcherClass KiranSnWatcherClass;
typedef struct _KiranSnWatcherPrivate KiranSnWatcherPrivate;

struct _KiranSnWatcher
{
    KiranSnWatcherGenSkeleton parent;

    KiranSnWatcherPrivate *priv;
};

struct _KiranSnWatcherClass
{
    KiranSnWatcherGenSkeletonClass parent_class;
};


GType kiran_sn_watcher_get_type (void);
KiranSnWatcher *kiran_sn_watcher_new (void);

G_END_DECLS

#endif /* __KIRAN_SN_WATCHER__ */
