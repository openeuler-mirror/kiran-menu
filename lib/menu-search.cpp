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

#include "lib/menu-search.h"

#include "lib/app.h"
#include "lib/base.h"
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
                                     AppKind kind,
                                     const AppVec &apps)
{
    AppVec match_apps;

    for (int i = 0; i < (int)apps.size(); ++i)
    {
        const auto &app = apps[i];

        if (((int)kind & (int)app->get_kind()) == 0)
            continue;

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