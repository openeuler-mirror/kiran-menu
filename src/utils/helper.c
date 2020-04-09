/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-09 23:01:16
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/utils/helper.c
 */
#include "src/utils/helper.h"

GList *list_remain_headn(GList *list, guint n, GDestroyNotify free_func) {
  guint len = g_list_length(list);
  if (n > len) n = len;
  for (guint i = 0; i < len - n; ++i) {
    GList *last = g_list_last(list);
    list = g_list_remove_link(list, last);
    g_list_free_full(last, free_func);
  }
  return list;
}