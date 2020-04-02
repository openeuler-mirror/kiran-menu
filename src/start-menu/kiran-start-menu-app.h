#pragma once

#include <gio/gio.h>

#include "src/start-menu/kiran-app-system.h"

#define KIRAN_TYPE_START_MENU_APP kiran_start_menu_app_get_type()

G_DECLARE_FINAL_TYPE(KiranStartMenuApp, kiran_start_menu_app, KIRAN,
                     START_MENU_APP, GApplication)

KiranStartMenuApp *kiran_start_menu_app_get(void);

KiranAppSystem *kiran_start_menu_get_app_system(KiranStartMenuApp *self);