/*
 * @Author       : tangjie02
 * @Date         : 2020-05-09 11:40:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-03 17:59:05
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/menu-common.cpp
 */

#include "lib/menu-common.h"

namespace Kiran
{
std::list<int32_t> read_as_to_list_quark(Glib::RefPtr<Gio::Settings> settings, const std::string &key)
{
    std::list<int32_t> new_value;

    auto value = settings->get_string_array(key);

    for (auto iter = value.begin(); iter != value.end(); ++iter)
    {
        Glib::Quark quark(*iter);
        new_value.push_back(quark.id());
    }
    return new_value;
}

bool write_list_quark_to_as(Glib::RefPtr<Gio::Settings> settings,
                            const std::string &key,
                            const std::list<int32_t> &value)
{
    /*GArray *trans_value = g_array_new(FALSE, FALSE, sizeof(gchar *));

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
    return g_settings_set_strv(settings, key, (const gchar *const *)new_value);*/
    return true;
}

}  // namespace Kiran