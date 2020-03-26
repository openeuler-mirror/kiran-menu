#pragma once

#include <gio/gio.h>
#include "src/start-menu/kiran-app-info.h"

#define KIRAN_TYPE_APP_SYSTEM (kiran_app_system_get_type())

G_DECLARE_FINAL_TYPE(KiranAppSystem, kiran_app_system, KIRAN, APP_SYSTEM,
                     GObject)

KiranAppInfo *kiran_app_system_lookup_app(KiranAppSystem *self,
                                          const char *app_id);

GList *kiran_app_system_get_registered_apps(KiranAppSystem *self);

gchar **kiran_app_system_get_all_sorted_apps(KiranAppSystem *self);

KiranAppSystem *kiran_app_system_get_default(void);
