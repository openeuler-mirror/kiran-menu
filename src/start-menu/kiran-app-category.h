#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_CATEGORY (kiran_app_category_get_type())
G_DECLARE_FINAL_TYPE(KiranAppCategory, kiran_app_category, KIRAN, APP_CATEGORY, GObject)

KiranAppCategory *kiran_app_category_get_new(void);

G_END_DECLS
