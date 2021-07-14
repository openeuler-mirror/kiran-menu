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

#include "lib/category.h"
#include "lib/menu-unit.h"

namespace Kiran
{
// 管理菜单中的APP的分类
class MenuCategory : public MenuUnit
{
public:
    MenuCategory();
    virtual ~MenuCategory();

    virtual void init();

    virtual void flush(const AppVec &apps);

    void flush_app(std::shared_ptr<App> app);

    bool add_app(const std::string &category_name, std::shared_ptr<App> app);

    bool del_app(const std::string &category_name, std::shared_ptr<App> app);

    std::vector<std::string> get_apps(const std::string &category_name);

    std::vector<std::string> get_names();

    std::map<std::string, std::vector<std::string>> get_all();

private:
    void store_categories();
    std::shared_ptr<Category> find_category(const std::string &category_name);

private:
    std::string file_path_;

    std::vector<std::shared_ptr<Category>> categories_;

    std::shared_ptr<CategoryNode> root_;
};

}  // namespace Kiran