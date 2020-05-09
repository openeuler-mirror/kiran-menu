/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 10:52:13
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/helper.c
 */
#include "lib/helper.h"

#include <string.h>

GList *list_remain_headn(GList *list, guint n, GDestroyNotify free_func)
{
    guint len = g_list_length(list);
    if (n > len) n = len;
    for (guint i = 0; i < len - n; ++i)
    {
        GList *last = g_list_last(list);
        list = g_list_remove_link(list, last);
        g_list_free_full(last, free_func);
    }
    return list;
}

gchar *str_trim(const gchar *str)
{
    gint len = strlen(str);
    gchar *new_str = g_malloc0(len + 1);

    if (!len)
    {
        return new_str;
    }

    gint start = 0;
    gint end = len - 1;
    while (g_ascii_isspace(str[start]) && start < len)
    {
        ++start;
    }
    while (g_ascii_isspace(str[end]) && end > 0)
    {
        --end;
    }

    for (gint i = start; i <= end; ++i)
    {
        new_str[i - start] = str[i];
    }
    return new_str;
}

gchar *str_tolower(const gchar *str)
{
    gchar *new_str = g_strdup(str);
    gint len = strlen(str);
    for (gint i = 0; i < len; ++i)
    {
        new_str[i] = g_ascii_tolower(str[i]);
    }
    return new_str;
}