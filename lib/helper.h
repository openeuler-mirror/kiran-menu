/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 22:54:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 13:59:41
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/helper.h
 */

#pragma once

#include <giomm.h>

#include <string>
#include <vector>

namespace Kiran
{
#define RETURN_VAL_IF_FALSE(cond, val) \
    {                                  \
        if (!(cond)) return val;       \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond) \
    {                         \
        if (!(cond)) return;  \
    }

#define RETURN_IF_TRUE(cond) \
    {                        \
        if (cond) return;    \
    }

#define CONTINUE_IF_FALSE(cond) \
    {                           \
        if (!(cond)) continue;  \
    }

#define CONTINUE_IF_TRUE(cond) \
    {                          \
        if (cond) continue;    \
    }

std::string str_trim(const std::string &str);

std::string str_tolower(const std::string &str);

std::vector<std::string> str_split(const std::string &s, const std::string &seperator);

using StringHash = uint32_t;

constexpr StringHash prime = 9973;
constexpr StringHash basis = 0xCBF29CE4ul;
constexpr StringHash hash_compile_time(char const *str, StringHash last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (StringHash)((*str ^ last_value) * (uint64_t)prime)) : last_value;
}
inline StringHash shash(char const *str)
{
    StringHash ret{basis};

    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

/// compile-time hash of string.
/// usage: "XXX"_hash
constexpr StringHash operator"" _hash(char const *p, size_t)
{
    return hash_compile_time(p);
}

}  // namespace Kiran