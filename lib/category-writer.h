/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-22 08:47:14
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category-writer.h
 */

#pragma once

#include <giomm.h>

#include "lib/category-node.h"

namespace Kiran
{
class CategoryWriter
{
public:
    CategoryWriter();
    virtual ~CategoryWriter();

    bool write_to_xml(std::shared_ptr<CategoryNode> node, const std::string &file_path);

private:
    bool write_rule(std::shared_ptr<CategoryNode> node, gint recurse_level, Gio::DataOutputStream *data_output);
    bool write_category(std::shared_ptr<CategoryNode> node, gint recurse_level, Gio::DataOutputStream *data_output);
    bool write_root(std::shared_ptr<CategoryNode> node, Gio::DataOutputStream *data_output);
};

}  // namespace Kiran