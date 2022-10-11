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

#pragma once

#include "lib/menu-unit.h"

namespace Kiran
{
// 用于菜单搜索功能
class MenuSearch : public MenuUnit
{
public:
    MenuSearch();
    virtual ~MenuSearch();

    virtual void init();

    virtual void flush(const AppVec &apps);

    AppVec search_by_keyword(const std::string &keyword,
                             bool ignore_case,
                             AppKind kind,
                             const AppVec &apps);

private:
    bool strstr_with_case(const std::string &a, const std::string &b, bool ignore_case);
};
}  // namespace Kiran
