#ifndef KIRAN_CATEGORY_ITEM_INCLUDE_H
#define KIRAN_CATEGORY_ITEM_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranCategoryItem KiranCategoryItem;
typedef struct {
    GtkEventBoxClass parent_class;
} KiranCategoryItemClass;

#define KIRAN_TYPE_CATEGORY_ITEM    kiran_category_item_get_type()
#define KIRAN_IS_CATEGORY_ITEM(o)   G_TYPE_CHECK_INSTANCE_TYPE(o, KIRAN_TYPE_CATEGORY_ITEM)
#define KIRAN_CATEGORY_ITEM(o)      G_TYPE_CHECK_INSTANCE_CAST(o, KIRAN_TYPE_CATEGORY_ITEM, KiranCategoryItem)

GType kiran_category_item_get_type(void);

KiranCategoryItem *kiran_category_item_new(const gchar *name, gboolean clickable);
const char* kiran_category_item_get_category_name(KiranCategoryItem *item);

#endif