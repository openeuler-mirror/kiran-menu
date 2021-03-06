/*
 * @Author       : tangjie02
 * @Date         : 2020-05-09 11:40:36
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-08 13:46:35
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
    std::list<Glib::ustring> new_value;

    for (auto iter = value.begin(); iter != value.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)*iter);
        Glib::ustring desktop_id = query_quark;
        new_value.push_back(desktop_id);
    }
    return settings->set_string_array(key, new_value);
}

}  // namespace Kiran