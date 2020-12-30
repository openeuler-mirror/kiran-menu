/**
 * @file log.h
 * @brief 日志接口
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#pragma once

#include <glib.h>
#include "helper.h"

#if defined (__GNUC__)
#  define __FUNC__     ((const char*) (__PRETTY_FUNCTION__))
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 19901L
#  define __FUNC__     ((const char*) (__func__))
#else
#  define __FUNC__     ((const char*) (__FUNCTION__))
#endif

#undef __FUNC__
#define __FUNC__ __func__

#define LOG_FATAL(fmt, ...)                                                                          \
    do                                                                                               \
    {                                                                                                \
        g_critical("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                                          \
    do                                                                                               \
    {                                                                                                \
        g_error("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_CRITICAL(fmt, ...)                                                                       \
    do                                                                                               \
    {                                                                                                \
        g_critical("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_WARNING(fmt, ...)                                                                        \
    do                                                                                             \
    {                                                                                              \
        g_warning("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_MESSAGE(fmt, ...)                                                                        \
    do                                                                                             \
    {                                                                                              \
        g_message("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_INFO(fmt, ...)                                                                           \
    do                                                                                             \
    {                                                                                             \
        g_info("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define LOG_DEBUG(fmt, ...)                                                                          \
    do                                                                                            \
    {                                                                                             \
        g_debug("%s: " fmt, __FUNC__, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE_START(fmt, ...)                                                             \
    do                                                                                             \
    {                                                                                             \
        LOG_DEBUG("START " fmt, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE_END(fmt, ...)                                                               \
    do                                                                                             \
    {                                                                                                \
       LOG_DEBUG("END " fmt, ##__VA_ARGS__); \
    } while (0)

#define SETTINGS_PROFILE(fmt, ...)                                                              \
        SETTINGS_PROFILE_START(fmt, ##__VA_ARGS__);    \
        SCOPE_EXIT({ SETTINGS_PROFILE_END(fmt, ##__VA_ARGS__); })
