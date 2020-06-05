/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 09:43:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-03 16:57:07
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category.h
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

    const std::string &get_name() { return this->name; }

    const std::string &get_icon() { return this->icon; }

    gboolean get_repeat() { return this->repeat; }

    std::vector<std::string> get_apps();

    void clear_apps();

    void add_app(std::shared_ptr<App> app);

    void del_app(std::shared_ptr<App> app);

    gboolean match_add_app(std::shared_ptr<App> app);

    gboolean add_rule_include_app(std::shared_ptr<App> app);

    gboolean add_rule_exclude_app(std::shared_ptr<App> app);

   private:
    gboolean match_desktop_category(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    gboolean match_desktop_id(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    gboolean match_rule(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app);
    gboolean match_app(std::shared_ptr<App> app);

    gboolean delete_node_with_desktop_id(std::shared_ptr<CategoryNode> node, const std::string &desktop_id);

   private:
    std::string name;
    std::string icon;

    gboolean repeat;

    std::shared_ptr<CategoryNode> node;

    std::map<uint32_t, uint32_t> category_apps;
};

}  // namespace Kiran