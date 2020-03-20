#pragma once

#include <gio/gio.h>

#define KIRAN_TYPE_APP_SYSTEM (kiran_app_system_get_type ())

G_DECLARE_FINAL_TYPE (KiranAppSystem, kiran_app_system, KIRAN, APP_SYSTEM, GObject)

KiranAppSystem *kiran_app_system_get_new ();

