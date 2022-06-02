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

#pragma once

#include <giomm.h>

#include <cinttypes>
#include <string>
#include <vector>

namespace Kiran
{
#define RETURN_VAL_IF_FALSE(cond, val)             \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return val;                            \
        }                                          \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond)                      \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return;                                \
        }                                          \
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

#define RET_WRAP_NULL(expr)       \
    {                             \
        auto _ret = expr;         \
        if (_ret == NULL)         \
        {                         \
            return std::string(); \
        }                         \
        return _ret;              \
    }

#define CONNECTION(text1, text2) text1##text2
#define CONNECT(text1, text2) CONNECTION(text1, text2)

class Defer
{
public:
    Defer(std::function<void(std::string)> func, std::string fun_name) : func_(func),
                                                                         fun_name_(fun_name) {}
    ~Defer() { func_(fun_name_); }

private:
    std::function<void(std::string)> func_;
    std::string fun_name_;
};

// helper macro for Defer class
#define SCOPE_EXIT(block) Defer CONNECT(_defer_, __LINE__)([&](std::string _arg_function) block, __FUNCTION__)

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

/// find characters in a string.
size_t find_chars(const std::string &s, const char *chs, size_t char_cnt, size_t off);
template <size_t _Size>
inline size_t find_chars(const std::string &s, const char (&chs)[_Size], size_t off = 0)
{
    // _Size - 1 will exclude the '\0'
    return find_chars(s, chs, _Size - 1, off);
}

std::string get_mainname(const std::string &file_name);

}  // namespace Kiran