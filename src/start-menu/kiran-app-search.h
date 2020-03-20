#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_SEARCH (kiran_app_search_get_type())
G_DECLARE_FINAL_TYPE(KiranAppSearch, kiran_app_search, KIRAN, APP_SEARCH, GObject)

KiranAppSearch *kiran_app_search_get_new(void);

G_END_DECLS
