/*
 * @Author       : tangjie02
 * @Date         : 2020-06-04 13:34:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 13:34:59
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-menu-common.h
 */
#pragma once
#include <gio/gio.h>

#include "lib/app.h"
#include "lib/menu-skeleton.h"

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
