/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:29:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 16:27:08
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category-node.cpp
 */

#include "lib/category-node.h"

namespace Kiran
{
CategoryNode::CategoryNode(CategoryNodeType type)
{
    this->type = type;
}

CategoryNode::~CategoryNode()
{
    this->next = nullptr;
    this->children = nullptr;
}

void CategoryNode::insert_after(std::shared_ptr<CategoryNode> new_sibling)
{
    g_return_if_fail(new_sibling->parent.lock() == nullptr);

    auto parent_lck = this->parent.lock();
    g_return_if_fail(parent_lck != nullptr);

    new_sibling->prev = this->shared_from_this();
    new_sibling->next = this->next;

    this->next = new_sibling;
    if (new_sibling->next)
    {
        new_sibling->next->prev = new_sibling;
    }
    else
    {
        parent_lck->last = new_sibling;
    }

    new_sibling->parent = parent_lck;
}

void CategoryNode::append_child(std::shared_ptr<CategoryNode> sub_node)
{
    auto last_lck = this->last.lock();
    if (last_lck)
    {
        last_lck->insert_after(sub_node);
    }
    else
    {
        this->children = sub_node;
        this->last = sub_node;
        sub_node->parent = this->shared_from_this();
    }
}

bool CategoryNode::has_child_of_type(CategoryNodeType type)
{
    auto iter = this->children;
    for (auto iter = this->children; iter; iter = iter->next)
    {
        if (iter->type == type)
            return true;
    }
    return false;
}

std::shared_ptr<CategoryNode> CategoryNode::steal()
{
    auto self = this->shared_from_this();
    auto parent_lck = this->parent.lock();
    auto prev_lck = this->prev.lock();

    if (parent_lck)
    {
        if (parent_lck->children == self)
        {
            parent_lck->children = self->next;
        }

        if (parent_lck->last.lock() == self)
        {
            parent_lck->last = self->prev;
        }
    }

    if (prev_lck)
    {
        prev_lck->next = this->next;
    }

    if (this->next)
    {
        this->next->prev = this->prev;
    }

    this->next = nullptr;
    return self;
}

}  // namespace Kiran