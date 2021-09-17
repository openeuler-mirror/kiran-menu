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

#include "lib/category-node.h"

namespace Kiran
{
CategoryNode::CategoryNode(CategoryNodeType type) : type_(type)
{
}

CategoryNode::~CategoryNode()
{
    this->next_ = nullptr;
    this->children_ = nullptr;
}

void CategoryNode::insert_after(std::shared_ptr<CategoryNode> new_sibling)
{
    g_return_if_fail(new_sibling->parent_.lock() == nullptr);

    auto parent_lck = this->parent_.lock();
    g_return_if_fail(parent_lck != nullptr);

    new_sibling->prev_ = this->shared_from_this();
    new_sibling->next_ = this->next_;

    this->next_ = new_sibling;
    if (new_sibling->next_)
    {
        new_sibling->next_->prev_ = new_sibling;
    }
    else
    {
        parent_lck->last_ = new_sibling;
    }

    new_sibling->parent_ = parent_lck;
}

void CategoryNode::append_child(std::shared_ptr<CategoryNode> sub_node)
{
    auto last_lck = this->last_.lock();
    if (last_lck)
    {
        last_lck->insert_after(sub_node);
    }
    else
    {
        this->children_ = sub_node;
        this->last_ = sub_node;
        sub_node->parent_ = this->shared_from_this();
    }
}

bool CategoryNode::has_child_of_type(CategoryNodeType type)
{
    auto iter = this->children_;
    for (auto iter = this->children_; iter; iter = iter->next_)
    {
        if (iter->type_ == type)
            return true;
    }
    return false;
}

std::shared_ptr<CategoryNode> CategoryNode::steal()
{
    auto self = this->shared_from_this();
    auto parent_lck = this->parent_.lock();
    auto prev_lck = this->prev_.lock();

    if (parent_lck)
    {
        if (parent_lck->children_ == self)
        {
            parent_lck->children_ = self->next_;
        }

        if (parent_lck->last_.lock() == self)
        {
            parent_lck->last_ = self->prev_;
        }
    }

    if (prev_lck)
    {
        prev_lck->next_ = this->next_;
    }

    if (this->next_)
    {
        this->next_->prev_ = this->prev_;
    }

    this->next_ = nullptr;
    return self;
}

}  // namespace Kiran