/*
 * @Author       : tangjie02
 * @Date         : 2020-05-09 11:40:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 14:13:46
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-common.c
 */

#include "lib/kiran-menu-common.h"

GList *read_as_to_list_quark(GSettings *settings, const gchar *key)
{
    g_auto(GStrv) value = g_settings_get_strv(settings, key);

    GList *new_value = NULL;

    for (gint i = 0; value[i] != NULL; ++i)
    {
        GQuark quark = g_quark_from_string(value[i]);
        new_value = g_list_append(new_value, GUINT_TO_POINTER(quark));
    }

    return new_value;
}

gboolean write_list_quark_to_as(GSettings *settings, const gchar *key, GList *value)
{
    GArray *trans_value = g_array_new(FALSE, FALSE, sizeof(gchar *));

    for (GList *l = value; l != NULL; l = l->next)
    {
        GQuark quark = GPOINTER_TO_UINT(l->data);
        const gchar *str = g_quark_to_string(quark);
        gchar *dup_str = g_strdup(str);
        g_array_append_val(trans_value, dup_str);
    }

    char *null = NULL;
    g_array_append_val(trans_value, null);

    g_auto(GStrv) new_value = (gchar **)g_array_free(trans_value, FALSE);
    return g_settings_set_strv(settings, key, (const gchar *const *)new_value);
}