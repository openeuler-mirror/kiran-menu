/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "lib/common.h"

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
    // g_print("key: %s value: %d\n", key.c_str(), new_value.size());
    return settings->set_string_array(key, new_value);
}

}  // namespace Kiran