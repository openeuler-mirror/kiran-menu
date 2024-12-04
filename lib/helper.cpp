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

#include "lib/helper.h"

#include <algorithm>
#include <cstring>

namespace Kiran
{
std::string str_trim(const std::string &str)
{
    std::string new_str = str;
    if (new_str.empty())
    {
        return new_str;
    }
    new_str.erase(0, new_str.find_first_not_of(" "));
    new_str.erase(new_str.find_last_not_of(" ") + 1);
    return new_str;
}

std::string str_tolower(const std::string &str)
{
    std::string new_str = str;
    std::transform(new_str.begin(), new_str.end(), new_str.begin(), tolower);
    return new_str;
}

std::vector<std::string> str_split(const std::string &s, const std::string &seperator)
{
    std::vector<std::string> result;

    int i = 0;

    while (i != (int)s.size())
    {
        int flag = 0;
        while (i != (int)s.size() && flag == 0)
        {
            flag = 1;
            for (int x = 0; x < (int)seperator.size(); ++x)
            {
                if (s[i] == seperator[x])
                {
                    ++i;
                    flag = 0;
                    break;
                }
            }
        }

        flag = 0;
        int j = i;
        while (j != (int)s.size() && flag == 0)
        {
            for (int x = 0; x < (int)seperator.size(); ++x)
            {
                if (s[j] == seperator[x])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
            {
                ++j;
            }
        }
        if (i != j)
        {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}

size_t find_chars(const std::string &s, const char *chs, size_t char_cnt, size_t off)
{
    for (size_t p = off; p < s.length(); p++)
    {
        auto c = s[p];
        for (size_t i = 0; i < char_cnt; i++)
        {
            if (c == chs[i])
            {
                return p;
            }
        }
    }
    return std::string::npos;
}

std::string get_mainname(const std::string &file_name)
{
    auto pos = file_name.rfind('.');
    if (pos == std::string::npos)
    {
        return file_name;
    }
    if (find_chars(file_name, "/\\", pos) != std::string::npos)
    {
        return file_name;
    }
    return file_name.substr(0, pos);
}

}  // namespace Kiran