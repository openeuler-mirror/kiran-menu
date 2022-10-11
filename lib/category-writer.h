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