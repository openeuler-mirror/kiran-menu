/*
 * @Author       : tangjie02
 * @Date         : 2020-05-07 09:43:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 16:15:03
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category.cpp
 */

#include "lib/category.h"

#include "lib/helper.h"

namespace Kiran
{
Category::Category(std::shared_ptr<CategoryNode> node) : repeat(TRUE)
{
    this->node = node;

    if (node->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY)
    {
        g_warning("KiranCategory init need the node which type must be CATEGORY_NODE_TYPE_CATEGORY.");
    }

    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
    {
        switch (iter->get_type())
        {
            case CategoryNodeType::CATEGORY_NODE_TYPE_NAME:
                this->name = iter->get_content();
                break;
            case CategoryNodeType::CATEGORY_NODE_TYPE_ICON:
                this->icon = iter->get_content();
                break;
            case CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT:
            {
                if (iter->get_content() == "true")
                {
                    this->repeat = TRUE;
                }
                else if (iter->get_content() == "false")
                {
                    this->repeat = FALSE;
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
    for (auto iter = this->category_apps.begin(); iter != this->category_apps.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)(iter->first));
        Glib::ustring desktop_id = query_quark;
        apps.push_back(desktop_id.raw());
    }
    return apps;
}

void Category::clear_apps()
{
    this->category_apps.clear();
}

void Category::add_app(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);

    this->category_apps[quark.id()] = TRUE;
}

void Category::del_app(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);
    this->category_apps.erase(quark.id());
}

gboolean Category::match_add_app(std::shared_ptr<App> app)
{
    if (match_app(app))
    {
        add_app(app);
        return TRUE;
    }
    return FALSE;
}

gboolean Category::add_rule_include_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node != NULL, FALSE);

    gboolean rule_change = FALSE;

    // remove desktop_id from <exclude> element
    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
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
            rule_change = TRUE;
            break;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    gboolean match_result = FALSE;
    std::shared_ptr<CategoryNode> include_node;

    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
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
                match_result = TRUE;
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
            this->node->append_child(include_node);
        }
        auto desktop_id_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->set_content(app->get_desktop_id());
        include_node->append_child(desktop_id_node);
        rule_change = TRUE;
    }

    return rule_change;
}

gboolean Category::add_rule_exclude_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node != NULL, FALSE);

    std::shared_ptr<CategoryNode> exclude_node;

    // check whether desktop_id exist in <exclude> element
    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE)
        {
            exclude_node = iter;
        }

        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE &&
            match_rule(iter, app))
        {
            return FALSE;
        }
    }

    // check whether desktop_id exist in <include> or <logic> element
    gboolean match_result = FALSE;
    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
            iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = TRUE;
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
            this->node->append_child(exclude_node);
        }
        auto desktop_id_node = std::make_shared<CategoryNode>(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
        desktop_id_node->set_content(app->get_desktop_id());
        exclude_node->append_child(desktop_id_node);
        return TRUE;
    }
    return FALSE;
}

gboolean Category::match_desktop_category(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    auto categories = app->get_categories();

    if (categories.length() == 0)
    {
        return FALSE;
    }

    auto strv = str_split(categories, ";");
    for (gint i = 0; i < strv.size(); ++i)
    {
        //g_print("%s:---%s---%s\n", kiran_app_get_desktop_id(app), strv[i], desktop_category);
        if (g_ascii_strncasecmp(strv[i].c_str(), node->get_content().c_str(), node->get_content().length() + 1) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

gboolean Category::match_desktop_id(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    auto &app_desktop_id = app->get_desktop_id();

    if (app_desktop_id == node->get_content())
    {
        return TRUE;
    }

    return FALSE;
}

gboolean Category::match_rule(std::shared_ptr<CategoryNode> node, std::shared_ptr<App> app)
{
    gboolean match_result = FALSE;

    switch (node->get_type())
    {
        case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
            return match_desktop_category(node, app);

        case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID:
            return match_desktop_id(node, app);

        case CategoryNodeType::CATEGORY_NODE_TYPE_ALL:
            return TRUE;

        default:
        {
            gboolean match_finish = FALSE;
            for (auto iter = node->get_children(); iter && !match_finish; iter = iter->get_next())
            {
                gboolean sub_result = match_rule(iter, app);
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
                            match_finish = TRUE;
                        }
                        break;
                    case CategoryNodeType::CATEGORY_NODE_TYPE_AND:
                        match_result = (match_result & sub_result);
                        if (!match_result)
                        {
                            match_finish = TRUE;
                        }
                        break;
                    case CategoryNodeType::CATEGORY_NODE_TYPE_NOT:
                        match_result = (!sub_result);
                        match_finish = TRUE;
                        break;
                }
            }
            return match_result;
        }
    }
}  // namespace Kiran

gboolean Category::match_app(std::shared_ptr<App> app)
{
    g_return_val_if_fail(this->node != nullptr, FALSE);

    gboolean match_result = FALSE;

    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
            iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE)
        {
            if (match_rule(iter, app))
            {
                match_result = TRUE;
                break;
            }
        }
    }

    for (auto iter = this->node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE)
        {
            if (match_result && match_rule(iter, app))
            {
                match_result = FALSE;
                break;
            }
        }
    }
    return match_result;
}

gboolean Category::delete_node_with_desktop_id(std::shared_ptr<CategoryNode> node, const std::string &desktop_id)
{
    g_return_val_if_fail(node != NULL, FALSE);

    for (auto iter = node->get_children(); iter; iter = iter->get_next())
    {
        if (iter->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID &&
            desktop_id == iter->get_content())
        {
            iter->steal();
            return TRUE;
        }
    }
    return FALSE;
}

}  // namespace Kiran