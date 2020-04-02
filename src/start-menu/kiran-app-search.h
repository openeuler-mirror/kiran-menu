#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-app-system.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_SEARCH (kiran_app_search_get_type())
G_DECLARE_FINAL_TYPE(KiranAppSearch, kiran_app_search, KIRAN, APP_SEARCH,
                     GObject)

KiranAppSearch *kiran_app_search_get_new(KiranAppSystem *app_system);

G_END_DECLS
