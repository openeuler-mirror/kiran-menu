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