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