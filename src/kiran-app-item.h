#ifndef KIRAN_APP_ITEM_INCLUDE_H
#define KIRAN_APP_ITEM_INCLUDE_H

#include <gtk/gtk.h>
#include <kiran-app.h>

typedef struct _KiranAppItem KiranAppItem;
typedef struct {
    GtkEventBoxClass parent_class;
} KiranAppItemClass;

#define KIRAN_TYPE_APP_ITEM    kiran_app_item_get_type()
#define KIRAN_IS_APP_ITEM(o)   G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_APP_ITEM)
#define KIRAN_APP_ITEM(o)      G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_APP_ITEM, KiranAppItem)

GType kiran_app_item_get_type();
KiranAppItem *kiran_app_item_new(KiranApp *app);

#endif  //KIRAN_APP_ITEM_INCLUDE_H