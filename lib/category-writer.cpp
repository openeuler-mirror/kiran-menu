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

#include "lib/category-writer.h"

#include "lib/base.h"

namespace Kiran
{
CategoryWriter::CategoryWriter()
{
}

CategoryWriter::~CategoryWriter()
{
}

bool CategoryWriter::write_to_xml(std::shared_ptr<CategoryNode> node, const std::string &file_path)
{
    Glib::RefPtr<Gio::FileOutputStream> output;
    std::string error;

    auto config_file = Gio::File::create_for_path(file_path);

    if (!config_file)
    {
        KLOG_WARNING("file %s create fail.\n", file_path.c_str());
        return false;
    }

    try
    {
        output = config_file->replace();
    }
    catch (const Gio::Error &e)
    {
        KLOG_WARNING("Could not save menu category data: %s", e.what().c_str());
        return false;
    }

    auto buffered_output = Gio::BufferedOutputStream::create(output);
    auto data_output = Gio::DataOutputStream::create(buffered_output);

    try
    {
        write_root(node, data_output.get());
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("failed to save menu category data: %s", e.what().c_str());
        return false;
    }
    return true;
}

#define PUT_STR_TO_CATEGORY_FILE(str)  \
    if (!data_output->put_string(str)) \
    {                                  \
        return false;                  \
    }

static bool write_prefix_space(gint recurse_level, Gio::DataOutputStream *data_output)
{
    for (gint i = 0; i < recurse_level; ++i)
    {
        PUT_STR_TO_CATEGORY_FILE("    ");
    }
    return true;
}

#define PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, str) \
    {                                                         \
        if (!write_prefix_space(recurse_level, data_output))  \
        {                                                     \
            return false;                                     \
        }                                                     \
        if (!data_output->put_string(str))                    \
        {                                                     \
            return false;                                     \
        }                                                     \
    }

bool CategoryWriter::write_rule(std::shared_ptr<CategoryNode> node, gint recurse_level, Gio::DataOutputStream *data_output)
{
    g_return_val_if_fail(node != nullptr, false);

    for (auto iter = node->get_children(); iter; iter = iter->get_next())
    {
        switch (iter->get_type())
        {
        case CategoryNodeType::CATEGORY_NODE_TYPE_ALL:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<all> </all>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_AND:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<and>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</and>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_OR:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<or>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</or>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_NOT:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<not>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</not>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<desktop_id> ");
            PUT_STR_TO_CATEGORY_FILE(iter->get_content());
            PUT_STR_TO_CATEGORY_FILE(" </desktop_id>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<desktop_category> ");
            PUT_STR_TO_CATEGORY_FILE(iter->get_content());
            PUT_STR_TO_CATEGORY_FILE(" </desktop_category>\n");
            break;
        default:
            break;
        }
    }
    return true;
}

bool CategoryWriter::write_category(std::shared_ptr<CategoryNode> node, gint recurse_level, Gio::DataOutputStream *data_output)
{
    g_return_val_if_fail(node != nullptr, false);
    g_return_val_if_fail(node->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY, false);

    PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, "<category>\n");

    for (auto iter = node->get_children(); iter; iter = iter->get_next())
    {
        switch (iter->get_type())
        {
        case CategoryNodeType::CATEGORY_NODE_TYPE_NAME:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<name> ");
            PUT_STR_TO_CATEGORY_FILE(iter->get_content());
            PUT_STR_TO_CATEGORY_FILE(" </name>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_ICON:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<icon> ");
            PUT_STR_TO_CATEGORY_FILE(iter->get_content());
            PUT_STR_TO_CATEGORY_FILE(" </icon>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<repeat> ");
            PUT_STR_TO_CATEGORY_FILE(iter->get_content());
            PUT_STR_TO_CATEGORY_FILE(" </repeat>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<logic>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</logic>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<include>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</include>\n");
            break;
        case CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE:
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<exclude>\n");
            write_rule(iter, recurse_level + 1, data_output);
            PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</exclude>\n");
            break;
        default:
            break;
        }
    }

    PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, "</category>\n");
    return true;
}

bool CategoryWriter::write_root(std::shared_ptr<CategoryNode> node, Gio::DataOutputStream *data_output)
{
    PUT_STR_TO_CATEGORY_FILE("<?xml version=\"1.0\"?>\n\n<categories>");

    auto categories = node->get_children();

    if (categories)
    {
        for (auto category = categories->get_children(); category; category = category->get_next())
        {
            if (!write_category(category, 1, data_output))
            {
                return false;
            }
        }
    }

    PUT_STR_TO_CATEGORY_FILE("</categories>\n");

    return true;
}

}  // namespace Kiran
