#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_USAGE (kiran_app_usage_get_type())
G_DECLARE_FINAL_TYPE(KiranAppUsage, kiran_app_usage, KIRAN, APP_USAGE, GObject)

KiranAppUsage *kiran_app_usage_get_new();

G_END_DECLS
