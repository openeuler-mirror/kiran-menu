#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-app-system.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_CATEGORY (kiran_app_category_get_type())
G_DECLARE_FINAL_TYPE(KiranAppCategory, kiran_app_category, KIRAN, APP_CATEGORY,
                     GObject)

KiranAppCategory *kiran_app_category_get_new(KiranAppSystem *app_system);

void kiran_app_category_load(KiranAppCategory *self);

G_END_DECLS
