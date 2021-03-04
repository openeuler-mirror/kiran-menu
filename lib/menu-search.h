/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 20:58:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:17:56
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-2.0/lib/menu-search.h
 */
#pragma once

#include "lib/menu-unit.h"

namespace Kiran
{
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
