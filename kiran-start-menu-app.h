#pragma once

#include <gio/gio.h>

#define KIRAN_TYPE_START_MENU_APP kiran_start_menu_app_get_type()

G_DECLARE_FINAL_TYPE(KiranStartMenuApp, kiran_start_menu_app, KIRAN,
                     START_MENU_APP, GApplication)

KiranStartMenuApp *kiran_start_menu_app_get(void);

