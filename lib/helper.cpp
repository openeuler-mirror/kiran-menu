/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:57
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 14:15:19
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/helper.cpp
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

    while (i != s.size())
    {
        int flag = 0;
        while (i != s.size() && flag == 0)
        {
            flag = 1;
            for (int x = 0; x < seperator.size(); ++x)
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
        while (j != s.size() && flag == 0)
        {
            for (int x = 0; x < seperator.size(); ++x)
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
}