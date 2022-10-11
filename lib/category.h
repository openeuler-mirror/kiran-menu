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

#include <giomm.h>

#include <map>

#include "lib/app.h"
#include "lib/category-node.h"

namespace Kiran
{
class Category
{
public:
    Category(std::shared_ptr<CategoryNode> node);
    virtual ~Category();

    const std::string &get_name() { return this->name_; }

    const std::string &get_icon() { return this->icon_; }

    bool get_repeat() { return this->repeat_; }

    std::vector<std::string> get_apps();

    void clear_apps();

    void add_app(std::shared_ptr<App> app);

    void del_app(std::shared_ptr<App> app);

    bool match_add_app(std::shared_ptr<App> app);

    bool add_rule_include_app(std::shared_ptr<App> app);

    bool add_rule_exclude_app(std::shared_ptr<App> app);

private:
    bool match_desktop_category(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    bool match_desktop_id(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    bool match_rule(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    bool match_app(std::shared_ptr<App> app);

    bool delete_node_with_desktop_id(std::shared_ptr<CategoryNode> node, const std::string &desktop_id);

private:
    std::string name_;
    std::string icon_;

    bool repeat_;

    std::shared_ptr<CategoryNode> node_;

    std::map<uint32_t, uint32_t> category_apps_;
};

}  // namespace Kiran