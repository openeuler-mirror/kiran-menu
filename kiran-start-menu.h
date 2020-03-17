#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "kiran-start-menu-generated.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_START_MENU (kiran_start_menu_get_type())

G_DECLARE_FINAL_TYPE(KiranStartMenu, kiran_start_menu, KIRAN, START_MENU,
                     GObject)

KiranStartMenu *kiran_start_menu_new(void);

gboolean kiran_start_menu_dbus_register(KiranStartMenu *start_menu,
                                        GDBusConnection *connection,
                                        const char *object_path,
                                        GError **error);

void kiran_start_menu_dbus_unregister(KiranStartMenu *start_menu,
                                      GDBusConnection *connection,
                                      const char *object_path);

G_END_DECLS