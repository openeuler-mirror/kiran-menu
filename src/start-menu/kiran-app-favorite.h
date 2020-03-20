#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_FAVORITE (kiran_app_favorite_get_type())
G_DECLARE_FINAL_TYPE(KiranAppFavorite, kiran_app_favorite, KIRAN, APP_FAVORITE, GObject)

KiranAppFavorite *kiran_app_favorite_get_new(void);

G_END_DECLS
