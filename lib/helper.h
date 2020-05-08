/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 17:20:15
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/helper.h
 */

#pragma once

#include <gio/gio.h>

#define RETURN_VAL_IF_FALSE(cond, val) \
    {                                  \
        if (!(cond)) return val;       \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond) \
    {                         \
        if (!(cond)) return;  \
    }

#define RETURN_IF_TRUE(cond) \
    {                        \
        if (cond) return;    \
    }

#define CONTINUE_IF_FALSE(cond) \
    {                           \
        if (!(cond)) continue;  \
    }

#define CONTINUE_IF_TRUE(cond) \
    {                          \
        if (cond) continue;    \
    }

GList *list_remain_headn(GList *list, guint n, GDestroyNotify free_func);

gchar *str_trim(const gchar *str);