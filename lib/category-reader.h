/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 14:16:02
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/category-reader.h
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