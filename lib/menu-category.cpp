/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 16:39:11
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-category.cpp
 */
#include "lib/menu-category.h"

#include "lib/category-reader.h"
#include "lib/category-writer.h"
#include "lib/category.h"
#include "lib/helper.h"

namespace Kiran
{
MenuCategory::MenuCategory()
{
    this->file_path = "/usr/share/kiran-menu/com.unikylin.Kiran.MenuCategory.xml";

    std::unique_ptr<CategoryReader> reader(new CategoryReader());

    this->root = reader->create_from_xml(this->file_path);

    if (this->root && this->root->get_children())
    {
        auto iter = this->root->get_children()->get_children();
        for (; iter; iter = iter->get_next())
        {
            if (iter->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY)
            {
                g_warning("exist invalid node type: %u\n", iter->get_type());
                continue;
            }
            std::shared_ptr<Category> category(new Category(iter));
            if (category)
            {
                auto &name = category->get_name();
                if (find_category(name))
                {
                    g_warning("Multiple category exist same name: %s\n", name.c_str());
                    continue;
                }
                this->categories.push_back(category);
            }
        }
    }
}

MenuCategory::~MenuCategory()
{
}

void MenuCategory::flush(const AppVec &apps)
{
    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];
        category->clear_apps();
    }

    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];

        gboolean match_result = FALSE;

        for (int j = 0; j < this->categories.size(); ++j)
        {
            auto &category = this->categories[j];

            if (match_result && !category->get_repeat())
            {
                continue;
            }

            if (category->match_add_app(app))
            {
                match_result = TRUE;
            }
        }
    }
}

void MenuCategory::flush_app(std::shared_ptr<App> app)
{
    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];
        category->del_app(app);
    }

    gboolean match_result = FALSE;

    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];

        if (match_result && !category->get_repeat())
        {
            continue;
        }

        if (category->match_add_app(app))
        {
            match_result = TRUE;
        }
    }
}

gboolean MenuCategory::add_app(const std::string &category_name, std::shared_ptr<App> app)
{
    auto category = find_category(category_name);

    if (category)
    {
        if (category->add_rule_include_app(app))
        {
            flush_app(app);
            store_categories();
            return TRUE;
        }
    }
    return FALSE;
}

gboolean MenuCategory::del_app(const std::string &category_name, std::shared_ptr<App> app)
{
    auto category = find_category(category_name);

    if (category)
    {
        if (category->add_rule_exclude_app(app))
        {
            flush_app(app);
            store_categories();
            return TRUE;
        }
    }
    return FALSE;
}

std::vector<std::string> MenuCategory::get_apps(const std::string &category_name)
{
    auto category = find_category(category_name);

    if (category)
    {
        return category->get_apps();
    }
    return std::vector<std::string>();
}

std::vector<std::string> MenuCategory::get_names()
{
    std::vector<std::string> category_names;

    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];
        auto &name = category->get_name();
        category_names.push_back(name);
    }
    return category_names;
}

std::map<std::string, std::vector<std::string>> MenuCategory::get_all()
{
    std::map<std::string, std::vector<std::string>> categories;

    gchar *category_name;

    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];

        auto &category_name = category->get_name();

        categories[category_name] = category->get_apps();
    }
    return categories;
}

void MenuCategory::store_categories()
{
    std::unique_ptr<CategoryWriter> writer(new CategoryWriter());

    writer->write_to_xml(this->root, this->file_path);
}

std::shared_ptr<Category> MenuCategory::find_category(const std::string &category_name)
{
    for (int i = 0; i < this->categories.size(); ++i)
    {
        auto &category = this->categories[i];
        auto &name = category->get_name();

        if (name == category_name)
        {
            return category;
        }
    }
    return nullptr;
}

}  // namespace Kiran