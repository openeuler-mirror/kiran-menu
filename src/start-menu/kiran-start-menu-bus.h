#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "kiran-start-menu-generated.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_START_MENU_BUS (kiran_start_menu_bus_get_type())

G_DECLARE_FINAL_TYPE(KiranStartMenuBus, kiran_start_menu_bus, KIRAN,
                     START_MENU_BUS, GObject)

KiranStartMenuBus *kiran_start_menu_bus_new(void);

gboolean kiran_start_menu_bus_dbus_register(KiranStartMenuBus *start_menu,
                                            GDBusConnection *connection,
                                            const char *object_path,
                                            GError **error);

void kiran_start_menu_bus_dbus_unregister(KiranStartMenuBus *start_menu,
                                          GDBusConnection *connection,
                                          const char *object_path);

G_END_DECLS