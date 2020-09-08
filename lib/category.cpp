/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 09:43:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 15:57:21
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category.cpp
 */

#include "lib/category.h"

#include "lib/log.h"

namespace Kiran
{
Category::Category(std::shared_ptr<CategoryNode> node) : repeat_(true),
                                                         node_(node)
{
    if (node->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY)
    {
        LOG_WARNING("KiranCategory init need the node which type must be CATEGORY_NODE_TYPE_CATEGORY.");
    }

    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        switch (iter->get_type())
        {
        case CategoryNodeType::CATEGORY_NODE_TYPE_NAME:
            this->name_ = iter->get_content();
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_ICON:
            this->icon_ = iter->get_content();
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT:
        {
            if (iter->get_content() == "true")
            {
                this->repeat_ = true;
            }
            else if (iter->get_content() == "false")
            {
                this->repeat_ = false;
            }
        }
        break;
        }
    }
}

Category::~Category()
{
}

std::vector<std::string> Category::get_apps()
{
    std::vector<std::string> apps;
    for (auto iter = this->category_apps_.begin(); iter != this->category_apps_.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)(iter->first));
        Glib::ustring desktop_id = query_quark;
        apps.push_back(desktop_id.raw());
    }
    return apps;
}

void Category::clear_apps()
{
    this->category_apps_.clear();
}

void Category::add_app(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);

    this->category_apps_[quark.id()] = true;
}

void Category::del_app(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);
    this->category_apps_.erase(quark.id());
}

bool Category::match_add_app(std::shared_ptr<App> app)
{
    if (match_app(app))
    {
        add_app(app);
        return true;
    }
    return false;
}

bool Category::add_rule_include_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node_ != nullptr, false);

    bool rule_change = false;

    // remove desktop_id from <exclude> element
    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE &&
            match_rule(iter, app))
        {
            auto &desktop_id = app->get_desktop_id();
            delete_node_with_desktop_id(iter, desktop_id);
            if (!iter->get_children())
            {
                iter->steal();
            }
            rule_change = true;
            break;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    bool match_result = false;
    std::shared_ptr<CategoryNode> include_node;

    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            include_node = iter;
        }

        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
            iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = true;
                break;
            }
        }
    }

    // add desktop_id to <include> element if match_result equal to false
    if (!match_result)
    {
        if (!include_node)
        {
            include_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE);
            this->node_->append_child(include_node);
        }
        auto desktop_id_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->set_content(app->get_desktop_id());
        include_node->append_child(desktop_id_node);
        rule_change = true;
    }

    return rule_change;
}

bool Category::add_rule_exclude_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node_ != nullptr, false);

    std::shared_ptr<CategoryNode> exclude_node;

    // check whether desktop_id exist in <exclude> element
    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE)
        {
            exclude_node = iter;
        }

        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE &&
            match_rule(iter, app))
        {
            return false;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    bool match_result = false;
    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
            iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = true;
                break;
            }
        }
    }

    // add desktop_id to <exclude> element if match_result equal to true
    if (match_result)
    {
        if (!exclude_node)
        {
            exclude_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE);
            this->node_->append_child(exclude_node);
        }
        auto desktop_id_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->set_content(app->get_desktop_id());
        exclude_node->append_child(desktop_id_node);
        return true;
    }
    return false;
}

bool Category::match_desktop_category(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    auto categories = app->get_categories();

    if (categories.length() == 0)
    {
        return false;
    }

    auto strv = str_split(categories, ";");
    for (gint i = 0; i < strv.size(); ++i)
    {
        //g_print("%s:---%s---%s\n", kiran_app_get_desktop_id(app), strv[i], desktop_category);
        if (g_ascii_strncasecmp(strv[i].c_str(), node->get_content().c_str(), node->get_content().length() + 1) == 0)
        {
            return true;
        }
    }
    return false;
}

bool Category::match_desktop_id(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    auto &app_desktop_id = app->get_desktop_id();

    if (app_desktop_id == node->get_content())
    {
        return true;
    }

    return false;
}

bool Category::match_rule(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    bool match_result = false;

    switch (node->get_type())
    {
    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
        return match_desktop_category(node, app);

    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID:
        return match_desktop_id(node, app);

    case CategoryNodeType::CATEGORY_NODE_TYPE_ALL:
        return true;

    default:
    {
        bool match_finish = false;
        for (auto iter = node->get_children(); iter && !match_finish; iter = iter->get_next())
        {
            bool sub_result = match_rule(iter, app);
            // first match result
            if (iter == node->get_children())
            {
                match_result = sub_result;
            }

            switch (node->get_type())
            {
            case CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC:
            case CategoryNodeType::CATEGORY_NODE_TYPE_OR:
            case CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE:
            case CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE:
                match_result = (match_result | sub_result);
                if (match_result)
                {
                    match_finish = true;
                }
                break;
            case CategoryNodeType::CATEGORY_NODE_TYPE_AND:
                match_result = (match_result & sub_result);
                if (!match_result)
                {
                    match_finish = true;
                }
                break;
            case CategoryNodeType::CATEGORY_NODE_TYPE_NOT:
                match_result = (!sub_result);
                match_finish = true;
                break;
            }
        }
        return match_result;
    }
    }
}  // namespace Kiran

bool Category::match_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node_ != nullptr, false);

    bool match_result = false;

    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
            iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = true;
                break;
            }
        }
    }

    for (auto iter = this->node_->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE)
        {
            if (match_result && match_rule(iter, app))
            {
                match_result = false;
                break;
            }
        }
    }
    return match_result;
}

bool Category::delete_node_with_desktop_id(std::shared_ptr<CategoryNode> node, const std::string &desktop_id)
{
    g_return_val_if_fail(node != NULL, false);

    for (auto iter = node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID &&
            desktop_id == iter->get_content())
        {
            iter->steal();
            return true;
        }
    }
    return false;
}

}  // namespace Kiran