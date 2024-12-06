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

#include "lib/category-reader.h"

#include <sstream>

#include "lib/base.h"
#include "lib/category-node.h"

namespace Kiran
{
CategoryReader::CategoryReader() : root_(new CategoryNode(CategoryNodeType::CATEGORY_NODE_TYPE_ROOT))
{
    this->last_ = this->root_;
}

CategoryReader::~CategoryReader()
{
}

#define ELEMENT_IS(name) (strcmp(element_name.c_str(), (name)) == 0)

void CategoryReader::on_start_element(ParseContext &context,
                                      const Glib::ustring &element_name,
                                      const AttributeMap &attributes)
{
    if (ELEMENT_IS("categories"))
    {
        if (this->last_ == this->root_ &&
            this->root_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORIES))
        {
            throw Glib::MarkupError(Glib::MarkupError::PARSE,
                                    "Exist multiple <categories> elements in file.\n");
        }
        start_categories_element(context,
                                 element_name.c_str(),
                                 attributes);
    }
    else if (ELEMENT_IS("category"))
    {
        start_category_element(context,
                               element_name.c_str(),
                               attributes);
    }
    else if (this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY)
    {
        start_category_child_element(context,
                                     element_name.c_str(),
                                     attributes);
    }
    else if (this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC ||
             this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_ALL ||
             this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_AND ||
             this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_OR ||
             this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_NOT)
    {
        start_logic_child_element(context,
                                  element_name.c_str(),
                                  attributes);
    }
    else if (this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE ||
             this->last_->get_type() == CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE)
    {
        start_include_exclude_element(context,
                                      element_name.c_str(),
                                      attributes);
    }
    else
    {
        throw_error(context,
                    Glib::MarkupError::UNKNOWN_ELEMENT,
                    "Element <%s> may not appear in this context\n",
                    element_name.c_str());
    }
}

void CategoryReader::on_end_element(ParseContext &context, const Glib::ustring &element_name)
{
    g_assert(this->last_ != NULL);

    switch (this->last_->get_type())
    {
    case CategoryNodeType::CATEGORY_NODE_TYPE_NAME:
    case CategoryNodeType::CATEGORY_NODE_TYPE_ICON:
    case CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT:
    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID:
    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
        if (this->last_->get_content().length() == 0)
        {
            throw_error(context,
                        Glib::MarkupError::INVALID_CONTENT,
                        "Element <%s> is required to contain text and was empty\n",
                        element_name.c_str());
        }
        break;
    case CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY:
        if (!this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_NAME))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "<category> elements are required to contain a <name> element\n");
        }
        break;
    default:
        break;
    }

    this->last_ = this->last_->get_parent();
}

void CategoryReader::on_text(ParseContext &context, const Glib::ustring &text)
{
    switch (this->last_->get_type())
    {
    case CategoryNodeType::CATEGORY_NODE_TYPE_NAME:
    case CategoryNodeType::CATEGORY_NODE_TYPE_ICON:
    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID:
    case CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
        this->last_->set_content(str_trim(text));
        break;
    case CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT:
        this->last_->set_content(str_trim(text));
        if (this->last_->get_content() != "true" &&
            this->last_->get_content() != "false")
        {
            throw_error(context,
                        Glib::MarkupError::INVALID_CONTENT,
                        "the text in <repeat> must be true or false.");
            this->last_->set_content(std::string());
        }
        break;

    case CategoryNodeType::CATEGORY_NODE_TYPE_ROOT:
    case CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORIES:
    case CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY:
    case CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE:
    case CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE:
    case CategoryNodeType::CATEGORY_NODE_TYPE_ALL:
    case CategoryNodeType::CATEGORY_NODE_TYPE_AND:
    case CategoryNodeType::CATEGORY_NODE_TYPE_OR:
    case CategoryNodeType::CATEGORY_NODE_TYPE_NOT:
        if (!all_whitespace(text.c_str(), text.length()))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "No text is allowed inside element <%s>",
                        context.get_element().c_str());
        }
        break;
    default:
        break;
    }
}

std::shared_ptr<CategoryNode> CategoryReader::create_from_xml(const std::string &file_path)
{
    std::string error;
    g_autofree char *text = NULL;
    gsize length;

    text = NULL;
    length = 0;

    KLOG_DEBUG("Loading \"%s\" from disk\n", file_path.c_str());

    auto file = Gio::File::create_for_path(file_path);

    if (!file)
    {
        KLOG_WARNING("failed to create file: %s\n", file_path.c_str());
        return nullptr;
    }

    if (!file->load_contents(text, length))
    {
        KLOG_WARNING("Failed to load \"%s\"\n", file_path.c_str());
        return nullptr;
    }

    std::unique_ptr<ParseContext> context(new ParseContext(*this));

    try
    {
        context->parse(Glib::ustring(text, length));
    }
    catch (const Glib::MarkupError &e)
    {
        error = e.what().raw();
    }

    try
    {
        context->end_parse();
    }
    catch (const Glib::MarkupError &e)
    {
        error = e.what().raw();
    }

    if (error.length() > 0)
    {
        KLOG_WARNING("Error \"%s\" loading \"%s\"\n", error.c_str(), file_path.c_str());
    }

    return this->root_;
}

void CategoryReader::throw_error(ParseContext &context,
                                 Glib::MarkupError::Code error_code,
                                 const char *format,
                                 ...)
{
    auto line = context.get_line_number();
    auto ch = context.get_char_number();
    char buffer[256];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    std::ostringstream iss;
    iss << "Line " << line << "character " << ch << ": " << buffer;

    throw Glib::MarkupError(error_code, iss.str());
}

void CategoryReader::check_no_attributes(ParseContext &context,
                                         const char *element_name,
                                         const AttributeMap &attributes)
{
    if (attributes.size() > 0)
    {
        throw_error(context,
                    Glib::MarkupError::PARSE,
                    "cannot contain attribute on <%s> element in this context",
                    element_name);
    }
}

void CategoryReader::push_node(CategoryNodeType type)
{
    std::shared_ptr<CategoryNode> node(new CategoryNode(type));
    this->last_->append_child(node);
    this->last_ = node;
}

void CategoryReader::start_categories_element(ParseContext &context,
                                              const char *element_name,
                                              const AttributeMap &attributes)
{
    check_no_attributes(context, element_name, attributes);

    if (this->last_->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_ROOT)
    {
        throw_error(context,
                    Glib::MarkupError::PARSE,
                    "<categories> element can only appear at toplevel\n");
    }
    else
    {
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORIES);
    }
}

void CategoryReader::start_category_element(ParseContext &context,
                                            const char *element_name,
                                            const AttributeMap &attributes)
{
    check_no_attributes(context, element_name, attributes);

    if (this->last_->get_type() != CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORIES)
    {
        throw_error(context,
                    Glib::MarkupError::PARSE,
                    "<category> element can only appear below <categories> elements\n");
    }
    else
    {
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_CATEGORY);
    }
}

void CategoryReader::start_category_child_element(ParseContext &context,
                                                  const char *element_name,
                                                  const AttributeMap &attributes)

{
    check_no_attributes(context, element_name, attributes);

    switch (shash(element_name))
    {
    case "name"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_NAME))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <name> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_NAME);
    }
    break;
    case "icon"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_ICON))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <icon> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_ICON);
    }
    break;
    case "repeat"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <repeat> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_REPEAT);
    }
    break;
    case "logic"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <logic> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_LOGIC);
    }
    break;
    case "include"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <include> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_INCLUDE);
    }
    break;
    case "exclude"_hash:
    {
        if (this->last_->has_child_of_type(CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE))
        {
            throw_error(context,
                        Glib::MarkupError::PARSE,
                        "Multiple <exclude> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_EXCLUDE);
    }
    break;
    default:
        throw_error(context,
                    Glib::MarkupError::UNKNOWN_ELEMENT,
                    "Element <%s> may not appear below <category>\n",
                    element_name);
        break;
    }
}

void CategoryReader::start_logic_child_element(ParseContext &context,
                                               const char *element_name,
                                               const AttributeMap &attributes)
{
    check_no_attributes(context, element_name, attributes);

    switch (shash(element_name))
    {
    case "desktop_id"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
        break;
    case "desktop_category"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_CATEGORY);
        break;
    case "all"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_ALL);
        break;
    case "and"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_AND);
        break;
    case "or"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_OR);
        break;
    case "not"_hash:
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_NOT);
        break;
    default:
        throw_error(context,
                    Glib::MarkupError::UNKNOWN_ELEMENT,
                    "Element <%s> may not appear in this context\n",
                    element_name);
        break;
    }
}

void CategoryReader::start_include_exclude_element(ParseContext &context,
                                                   const char *element_name,
                                                   const AttributeMap &attributes)
{
    check_no_attributes(context, element_name, attributes);

    if (strcmp(element_name, "desktop_id") == 0)
    {
        push_node(CategoryNodeType::CATEGORY_NODE_TYPE_DESKTOP_ID);
    }
    else
    {
        throw_error(context,
                    Glib::MarkupError::UNKNOWN_ELEMENT,
                    "Element <%s> may not appear in this context\n",
                    element_name);
    }
}

gboolean CategoryReader::all_whitespace(const char *text,
                                        int text_len)
{
    const char *p;
    const char *end;

    p = text;
    end = text + text_len;

    while (p != end)
    {
        if (!g_ascii_isspace(*p))
            return FALSE;

        p = g_utf8_next_char(p);
    }

    return TRUE;
}

}  // namespace Kiran