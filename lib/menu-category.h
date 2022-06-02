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