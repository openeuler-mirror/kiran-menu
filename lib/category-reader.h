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