#pragma once
#include <gio/gio.h>

#include "src/start-menu/kiran-app.h"
#include "src/start-menu/kiran-menu-based.h"

#define CHECK_PROXY_CALL_ERR(result, error)                       \
    {                                                             \
        if (!result)                                              \
        {                                                         \
            g_printerr("proxy call error: %s\n", error->message); \
            g_error_free(error);                                  \
            return;                                               \
        }                                                         \
    }

#define CHECK_PROXY_CALL_ERR_WITH_RET(result, error, ret)         \
    {                                                             \
        if (!result)                                              \
        {                                                         \
            g_printerr("proxy call error: %s\n", error->message); \
            g_error_free(error);                                  \
            return ret;                                           \
        }                                                         \
    }
