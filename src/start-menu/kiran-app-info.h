#pragma once

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_APP_INFO (kiran_app_info_get_type())
G_DECLARE_FINAL_TYPE(KiranAppInfo, kiran_app_info, KIRAN, APP_INFO, GObject)

GDesktopAppInfo *kiran_app_info_get_desktop_app(KiranAppInfo *app);

char *kiran_app_info_get_name(KiranAppInfo *app);
char *kiran_app_info_get_desktop_id(KiranAppInfo *app);
const char *kiran_app_info_get_categories(KiranAppInfo *app);

KiranAppInfo *kiran_app_get_new(const char *app_id);

G_END_DECLS
