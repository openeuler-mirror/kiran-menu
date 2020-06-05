/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:30:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 13:39:26
 * @Description  : 管理菜单中的APP的分类
 * @FilePath     : /kiran-menu-2.0/lib/menu-category.h
 */
#pragma once

#include "lib/category.h"
#include "lib/menu-unit.h"

namespace Kiran
{
class MenuCategory : public MenuUnit
{
   public:
    MenuCategory();
    virtual ~MenuCategory();

    virtual void flush(const AppVec &apps);

    void flush_app(std::shared_ptr<App> app);

    gboolean add_app(const std::string &category_name, std::shared_ptr<App> app);

    gboolean del_app(const std::string &category_name, std::shared_ptr<App> app);

    std::vector<std::string> get_apps(const std::string &category_name);

    std::vector<std::string> get_names();

    std::map<std::string, std::vector<std::string>> get_all();

   private:
    void store_categories();
    std::shared_ptr<Category> find_category(const std::string &category_name);

   private:
    std::string file_path;

    std::vector<std::shared_ptr<Category>> categories;

    std::shared_ptr<CategoryNode> root;
};

}  // namespace Kiran