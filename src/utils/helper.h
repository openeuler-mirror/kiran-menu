/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:32:37
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/utils/helper.h
 */

#pragma once

#include <gio/gio.h>

#define RETURN_VAL_IF_FALSE(cond, val) \
  {                                    \
    if (!(cond)) return val;           \
  }

#define RETURN_VAL_IF_TRUE(cond, val) \
  {                                   \
    if (cond) return val;             \
  }

#define RETURN_IF_FALSE(cond) \
  {                           \
    if (!(cond)) return;      \
  }

#define RETURN_IF_TRUE(cond) \
  {                          \
    if (cond) return;        \
  }

#define CONTINUE_IF_FALSE(cond) \
  {                             \
    if (!(cond)) continue;      \
  }

#define CONTINUE_IF_TRUE(cond) \
  {                            \
    if (cond) continue;        \
  }

GList *list_remain_headn(GList *list, guint n, GDestroyNotify free_func);
