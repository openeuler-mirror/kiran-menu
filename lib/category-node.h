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

namespace Kiran
{
enum class CategoryNodeType
{
    CATEGORY_NODE_TYPE_UNKNOWN,
    CATEGORY_NODE_TYPE_ROOT,
    CATEGORY_NODE_TYPE_CATEGORIES,
    CATEGORY_NODE_TYPE_CATEGORY,
    CATEGORY_NODE_TYPE_NAME,
    CATEGORY_NODE_TYPE_ICON,
    CATEGORY_NODE_TYPE_REPEAT,
    CATEGORY_NODE_TYPE_LOGIC,
    CATEGORY_NODE_TYPE_ALL,
    CATEGORY_NODE_TYPE_AND,
    CATEGORY_NODE_TYPE_OR,
    CATEGORY_NODE_TYPE_NOT,
    CATEGORY_NODE_TYPE_DESKTOP_ID,
    CATEGORY_NODE_TYPE_DESKTOP_CATEGORY,
    CATEGORY_NODE_TYPE_INCLUDE,
    CATEGORY_NODE_TYPE_EXCLUDE,
};

class CategoryNode : public std::enable_shared_from_this<CategoryNode>
{
public:
    CategoryNode(CategoryNodeType type);
    virtual ~CategoryNode();

    std::shared_ptr<CategoryNode> get_next() { return this->next_; }
    std::shared_ptr<CategoryNode> get_children() { return this->children_; }
    std::shared_ptr<CategoryNode> get_parent() { return this->parent_.lock(); }
    const std::string& get_content() { return this->content_; }
    void set_content(const std::string& content) { this->content_ = content; }
    CategoryNodeType get_type() { return this->type_; }

    void insert_after(std::shared_ptr<CategoryNode> new_sibling);

    void append_child(std::shared_ptr<CategoryNode> sub_node);

    bool has_child_of_type(CategoryNodeType type);

    std::shared_ptr<CategoryNode> steal();

private:
    std::weak_ptr<CategoryNode> prev_;
    std::shared_ptr<CategoryNode> next_;
    std::weak_ptr<CategoryNode> parent_;
    std::shared_ptr<CategoryNode> children_;
    std::weak_ptr<CategoryNode> last_;

    std::string content_;
    CategoryNodeType type_;
};

}  // namespace Kiran