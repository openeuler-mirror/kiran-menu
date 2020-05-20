#ifndef KIRAN_SEARCH_ENTRY_INCLUDE_H
#define KIRAN_SEARCH_ENTRY_INCLUDE_H

#include <gtk/gtk.h>

typedef struct _KiranSearchEntry KiranSearchEntry;
typedef struct _KiranSearchEntryClass {
    GtkSearchEntryClass parent_class;
} KiranSearchEntryClass;

#define KIRAN_TYPE_SEARCH_ENTRY    kiran_search_entry_get_type()
#define KIRAN_IS_SEARCH_ENTRY(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, kiran_search_entry_get_type())
#define KIRAN_SEARCH_ENTRY(obj)    G_TYPE_CHECK_INSTANCE_CAST(obj, kiran_search_entry_get_type(), KiranSearchEntry)

GType kiran_search_entry_get_type();

KiranSearchEntry *kiran_search_entry_new(void);

#endif