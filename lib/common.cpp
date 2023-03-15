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
#include "lib/pinyin.h"

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

static wchar_t *
convert_chars_to_wchars(const std::string &contents)
{
    size_t mbs_len;
    wchar_t *wcs;

    mbs_len = mbstowcs(NULL, contents.c_str(), 0);
    if (mbs_len == (size_t)-1)
    {
        return NULL;
    }

    wcs = (wchar_t *)calloc(mbs_len + 1, sizeof(wchar_t));
    if (wcs == NULL)
    {
        return NULL;
    }

    if (mbstowcs(wcs, contents.c_str(), mbs_len + 1) == (size_t)-1)
    {
        free(wcs);
        return NULL;
    }

    return wcs;
}

static void
clear_space(char *pinyin)
{
    for (; *pinyin != '\0'; pinyin++)
    {
        if (*pinyin == ' ')
        {
            *pinyin = '\0';
        }
    }
}

std::list<std::string> convert_chinese_characters_to_pinyin(const std::string &contents)
{
    std::list<std::string> pinyin_list;
    wchar_t *wcs;
    int unicode;

    wcs = convert_chars_to_wchars(contents);

    if (wcs == NULL)
    {
        return pinyin_list;
    }

    for (auto wp = wcs; *wp != '\0'; wp++)
    {
        int unicode = *wp;

        if (unicode >= CHINESE_UNICODE_START &&
            unicode <= CHINESE_UNICODE_END)
        {
            char pinyin[CHINESE_UNICODE_MAX_LENGTH + 1] = {'\0'};
            int offset = 0;

            offset = (unicode - CHINESE_UNICODE_START) * CHINESE_UNICODE_MAX_LENGTH;

            if (offset + CHINESE_UNICODE_MAX_LENGTH <= sizeof(chinese_basic_dict))
            {
                memcpy(pinyin, chinese_basic_dict + offset, CHINESE_UNICODE_MAX_LENGTH);
                clear_space(pinyin);
                pinyin_list.push_back(pinyin);
            }
        }
    }

    free(wcs);

    return pinyin_list;
}

}  // namespace Kiran