/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:01:52
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 11:46:14
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-2.0/lib/menu-search.cpp
 */
#include "lib/menu-search.h"

#include "lib/app.h"
#include "lib/helper.h"

namespace Kiran
{
MenuSearch::MenuSearch()
{
}

MenuSearch::~MenuSearch()
{
}

void MenuSearch::init()
{
}

void MenuSearch::flush(const AppVec &apps)
{
}

AppVec MenuSearch::search_by_keyword(const std::string &keyword,
                                     bool ignore_case,
                                     const AppVec &apps)
{
    AppVec match_apps;

    for (int i = 0; i < apps.size(); ++i)
    {
        const auto &app = apps[i];

        auto &comment = app->get_comment();
        auto &locale_comment = app->get_locale_comment();
        auto &name = app->get_name();
        auto &locale_name = app->get_locale_name();

#define STRSTR_KEYWORD(a) (strstr_with_case(a, keyword, ignore_case))

        if (STRSTR_KEYWORD(name) || STRSTR_KEYWORD(locale_name) ||
            STRSTR_KEYWORD(comment) || STRSTR_KEYWORD(locale_comment))
        {
            match_apps.push_back(app);
        }

#undef STRSTR_KEYWORD
    }
    return match_apps;
}

bool MenuSearch::strstr_with_case(const std::string &a, const std::string &b, bool ignore_case)
{
    if (b.length() == 0)
    {
        return false;
    }

    if (std::search(a.begin(), a.end(), b.begin(), b.end()) != a.end())
    {
        return true;
    }

    if (ignore_case)
    {
        std::string a_lower = str_tolower(a);
        std::string b_lower = str_tolower(b);
        return (std::search(a_lower.begin(), a_lower.end(), b_lower.begin(), b_lower.end()) != a_lower.end());
    }

    return false;
}

}  // namespace Kiran