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

using namespace Glib::Markup;

namespace Kiran
{
class CategoryReader : public Parser
{
public:
    CategoryReader();
    virtual ~CategoryReader();

    virtual void on_start_element(ParseContext& context, const Glib::ustring& element_name, const AttributeMap& attributes);

    virtual void on_end_element(ParseContext& context, const Glib::ustring& element_name);

    virtual void on_text(ParseContext& context, const Glib::ustring& text);

    // virtual void on_passthrough(ParseContext& context, const Glib::ustring& passthrough_text);

    // virtual void on_error(ParseContext& context, const Glib::MarkupError& error);

    std::shared_ptr<CategoryNode> create_from_xml(const std::string& file_path);

private:
    void throw_error(ParseContext& context,
                     Glib::MarkupError::Code error_code,
                     const char* format,
                     ...);

    void check_no_attributes(ParseContext& context,
                             const char* element_name,
                             const AttributeMap& attributes);

    void push_node(CategoryNodeType type);

    void start_categories_element(ParseContext& context,
                                  const char* element_name,
                                  const AttributeMap& attributes);

    void start_category_element(ParseContext& context,
                                const char* element_name,
                                const AttributeMap& attributes);

    void start_category_child_element(ParseContext& context,
                                      const char* element_name,
                                      const AttributeMap& attributes);

    void start_logic_child_element(ParseContext& context,
                                   const char* element_name,
                                   const AttributeMap& attributes);

    void start_include_exclude_element(ParseContext& context,
                                       const char* element_name,
                                       const AttributeMap& attributes);

    gboolean all_whitespace(const char* text,
                            int text_len);

private:
    std::shared_ptr<CategoryNode> root_;
    std::shared_ptr<CategoryNode> last_;
};

}  // namespace Kiran