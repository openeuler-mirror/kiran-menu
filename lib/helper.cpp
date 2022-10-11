/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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