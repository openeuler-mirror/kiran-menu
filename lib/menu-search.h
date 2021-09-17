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
