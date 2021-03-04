/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:28:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 16:00:36
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-category.cpp
 */
#include "lib/menu-category.h"

#include "lib/category-reader.h"
#include "lib/category-writer.h"
#include "lib/category.h"
#include "lib/log.h"
#include "config.h"

namespace Kiran
{
MenuCategory::MenuCategory()
{
    this->file_path_ = PACKAGE_DATA_DIR "/com.unikylin.Kiran.MenuCategory.xml";
    std::unique_ptr<CategoryReader> reader(new CategoryReader());
    this->root_ = reader->create_from_xml(this->file_path_);
}

MenuCategory::~MenuCategory()
{
}

void MenuCategory::init()
{
    if (this->root_ && this->root_->get_children())
    {
        auto iter = this->root_->get_children()->get_children();
        for (; iter; iter = iter->get_next())
        {
            if (iter->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY)
            {
                LOG_WARNING("exist invalid node type: %u\n", iter->get_type());
                continue;
            }
            std::shared_ptr<Category> category(new Category(iter));
            if (category)
            {
                auto &name = category->get_name();
                if (find_category(name))
                {
                    LOG_WARNING("Multiple category exist same name: %s\n", name.c_str());
                    continue;
                }
                this->categories_.push_back(category);
            }
        }
    }
}

void MenuCategory::flush(const AppVec &apps)
{
    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];
        category->clear_apps();
    }

    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];

        bool match_result = false;

        /* 仅在开始菜单中显示系统应用 */
        if (app->get_kind() != AppKind::NORMAL)
            continue;

        for (int j = 0; j < this->categories_.size(); ++j)
        {
            auto &category = this->categories_[j];

            if (match_result && !category->get_repeat())
            {
                continue;
            }

            if (category->match_add_app(app))
            {
                match_result = true;
            }
        }
    }
}

void MenuCategory::flush_app(std::shared_ptr<App> app)
{
    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];
        category->del_app(app);
    }

    bool match_result = false;

    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];

        if (match_result && !category->get_repeat())
        {
            continue;
        }

        if (category->match_add_app(app))
        {
            match_result = true;
        }
    }
}

bool MenuCategory::add_app(const std::string &category_name, std::shared_ptr<App> app)
{
    auto category = find_category(category_name);

    if (category)
    {
        if (category->add_rule_include_app(app))
        {
            flush_app(app);
            store_categories();
            return true;
        }
    }
    return false;
}

bool MenuCategory::del_app(const std::string &category_name, std::shared_ptr<App> app)
{
    auto category = find_category(category_name);

    if (category)
    {
        if (category->add_rule_exclude_app(app))
        {
            flush_app(app);
            store_categories();
            return true;
        }
    }
    return false;
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

    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];
        auto &name = category->get_name();
        category_names.push_back(name);
    }
    return category_names;
}

std::map<std::string, std::vector<std::string>> MenuCategory::get_all()
{
    std::map<std::string, std::vector<std::string>> categories;

    gchar *category_name;

    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];

        auto &category_name = category->get_name();

        categories[category_name] = category->get_apps();
    }
    return categories;
}

void MenuCategory::store_categories()
{
    std::unique_ptr<CategoryWriter> writer(new CategoryWriter());

    writer->write_to_xml(this->root_, this->file_path_);
}

std::shared_ptr<Category> MenuCategory::find_category(const std::string &category_name)
{
    for (int i = 0; i < this->categories_.size(); ++i)
    {
        auto &category = this->categories_[i];
        auto &name = category->get_name();

        if (name == category_name)
        {
            return category;
        }
    }
    return nullptr;
}

}  // namespace Kiran
