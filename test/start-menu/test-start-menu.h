#pragma once
#include <gio/gio.h>
#include "src/start-menu/kiran-start-menu-generated.h"

#define DBUS_NAME "com.unikylin.Kiran.StartMenu"
#define DBUS_OBJECT_PATH "/com/unikylin/Kiran/StartMenu"
#define DBUS_OBJECT_MAIN_INTERFACE "com.unikylin.StartMenuS"
#define DBUS_OBJECT_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define CHECK_PROXY_CALL_ERR(result, error)                 \
  {                                                         \
    if (!result) {                                          \
      g_printerr("proxy call error: %s\n", error->message); \
      g_error_free(error);                                  \
      return;                                               \
    }                                                       \
  }

#define CHECK_PROXY_CALL_ERR_WITH_RET(result, error, ret)   \
  {                                                         \
    if (!result) {                                          \
      g_printerr("proxy call error: %s\n", error->message); \
      g_error_free(error);                                  \
      return ret;                                           \
    }                                                       \
  }
