/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:47
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-08 11:29:37
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-reader.c
 */

#include "lib/kiran-category-reader.h"

#include "lib/helper.h"
#include "lib/kiran-category-node.h"

struct _KiranCategoryReader
{
    GObject parent_instance;

    KiranCategoryNode *root;
    KiranCategoryNode *last;
};

G_DEFINE_TYPE(KiranCategoryReader, kiran_category_reader, G_TYPE_OBJECT);

#define ELEMENT_IS(name) (strcmp(element_name, (name)) == 0)

static void set_error(GError **err,
                      GMarkupParseContext *context,
                      int error_domain,
                      int error_code,
                      const char *format,
                      ...) G_GNUC_PRINTF(5, 6);

static void start_element_handler(GMarkupParseContext *context,
                                  const char *element_name,
                                  const char **attribute_names,
                                  const char **attribute_values,
                                  gpointer user_data,
                                  GError **error);

static void end_element_handler(GMarkupParseContext *context,
                                const char *element_name,
                                gpointer user_data,
                                GError **error);

static void text_handler(GMarkupParseContext *context,
                         const char *text,
                         gsize text_len,
                         gpointer user_data,
                         GError **error);

static GMarkupParser category_funcs = {
    start_element_handler,
    end_element_handler,
    text_handler,
    NULL,
    NULL};

static void set_error(GError **err,
                      GMarkupParseContext *context,
                      int error_domain,
                      int error_code,
                      const char *format,
                      ...)
{
    int line, ch;
    va_list args;
    char *str;

    g_markup_parse_context_get_position(context, &line, &ch);

    va_start(args, format);
    str = g_strdup_vprintf(format, args);
    va_end(args);

    g_set_error(err, error_domain, error_code,
                "Line %d character %d: %s",
                line, ch, str);

    g_free(str);
}

static gboolean
check_no_attributes(GMarkupParseContext *context,
                    const char *element_name,
                    const char **attribute_names,
                    const char **attribute_values,
                    GError **error)
{
    if (attribute_names[0] != NULL)
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                  "Attribute \"%s\" is invalid on <%s> element in this context",
                  attribute_names[0], element_name);
        return FALSE;
    }

    return TRUE;
}

static int has_child_of_type(KiranCategoryNode *node,
                             CategoryNodeType type)
{
    KiranCategoryNode *iter;

    iter = node->children;
    while (iter)
    {
        if (iter->type == type)
            return TRUE;

        iter = (iter->next == node->children) ? NULL : iter->next;
    }

    return FALSE;
}

static void push_node(KiranCategoryReader *reader,
                      CategoryNodeType type)
{
    KiranCategoryNode *node;

    node = kiran_category_node_get_new(type);
    kiran_category_node_append_child(reader->last, node);
    reader->last = node;
}

static void start_categories_element(KiranCategoryReader *reader,
                                     GMarkupParseContext *context,
                                     const char *element_name,
                                     const char **attribute_names,
                                     const char **attribute_values,
                                     GError **error)
{
    if (!check_no_attributes(context, element_name,
                             attribute_names, attribute_values,
                             error))
        return;

    if (reader->last->type != CATEGORY_NODE_TYPE_ROOT)
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                  "<categories> element can only appear at toplevel\n");
    }
    else
    {
        push_node(reader, CATEGORY_NODE_TYPE_CATEGORIES);
    }
}

static void start_category_element(KiranCategoryReader *reader,
                                   GMarkupParseContext *context,
                                   const char *element_name,
                                   const char **attribute_names,
                                   const char **attribute_values,
                                   GError **error)
{
    if (!check_no_attributes(context, element_name,
                             attribute_names, attribute_values,
                             error))
        return;

    if (reader->last->type != CATEGORY_NODE_TYPE_CATEGORIES)
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                  "<category> element can only appear below <categories> elements\n");
    }
    else
    {
        push_node(reader, CATEGORY_NODE_TYPE_CATEGORY);
    }
}

static void start_category_child_element(KiranCategoryReader *reader,
                                         GMarkupParseContext *context,
                                         const char *element_name,
                                         const char **attribute_names,
                                         const char **attribute_values,
                                         GError **error)
{
    if (!check_no_attributes(context, element_name,
                             attribute_names, attribute_values,
                             error))
        return;

    if (ELEMENT_IS("name"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_NAME))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <name> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_NAME);
    }
    else if (ELEMENT_IS("icon"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_ICON))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <icon> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_ICON);
    }
    else if (ELEMENT_IS("repeat"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_REPEAT))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <repeat> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_REPEAT);
    }
    else if (ELEMENT_IS("logic"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_LOGIC))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <logic> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_LOGIC);
    }
    else if (ELEMENT_IS("include"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_INCLUDE))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <include> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_INCLUDE);
    }
    else if (ELEMENT_IS("exclude"))
    {
        if (has_child_of_type(reader->last, CATEGORY_NODE_TYPE_EXCLUDE))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Multiple <exclude> elements in a <category> element is not allowed\n");
            return;
        }
        push_node(reader, CATEGORY_NODE_TYPE_EXCLUDE);
    }
    else
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                  "Element <%s> may not appear below <category>\n", element_name);
    }
}

static void start_logic_child_element(KiranCategoryReader *reader,
                                      GMarkupParseContext *context,
                                      const char *element_name,
                                      const char **attribute_names,
                                      const char **attribute_values,
                                      GError **error)
{
    if (!check_no_attributes(context, element_name,
                             attribute_names, attribute_values,
                             error))
        return;

    if (ELEMENT_IS("desktop_id"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_DESKTOP_ID);
    }
    else if (ELEMENT_IS("desktop_category"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_DESKTOP_CATEGORY);
    }
    else if (ELEMENT_IS("all"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_ALL);
    }
    else if (ELEMENT_IS("and"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_AND);
    }
    else if (ELEMENT_IS("or"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_OR);
    }
    else if (ELEMENT_IS("not"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_NOT);
    }
    else
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                  "Element <%s> may not appear in this context\n",
                  element_name);
    }
}

static void start_include_exclude_element(KiranCategoryReader *reader,
                                          GMarkupParseContext *context,
                                          const char *element_name,
                                          const char **attribute_names,
                                          const char **attribute_values,
                                          GError **error)
{
    if (!check_no_attributes(context, element_name,
                             attribute_names, attribute_values,
                             error))
        return;

    if (ELEMENT_IS("desktop_id"))
    {
        push_node(reader, CATEGORY_NODE_TYPE_DESKTOP_ID);
    }
    else
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                  "Element <%s> may not appear in this context\n",
                  element_name);
    }
}

static void start_element_handler(GMarkupParseContext *context,
                                  const char *element_name,
                                  const char **attribute_names,
                                  const char **attribute_values,
                                  gpointer user_data,
                                  GError **error)
{
    KiranCategoryReader *reader = KIRAN_CATEGORY_READER(user_data);

    if (ELEMENT_IS("categories"))
    {
        if (reader->last == reader->root && has_child_of_type(reader->root, CATEGORY_NODE_TYPE_CATEGORIES))
        {
            set_error(error, context,
                      G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                      "Exist multiple <categories> elements in file.\n");
            return;
        }
        start_categories_element(reader, context, element_name, attribute_names, attribute_values, error);
    }
    else if (ELEMENT_IS("category"))
    {
        start_category_element(reader, context, element_name, attribute_names, attribute_values, error);
    }
    else if (reader->last->type == CATEGORY_NODE_TYPE_CATEGORY)
    {
        start_category_child_element(reader, context, element_name, attribute_names, attribute_values, error);
    }
    else if (reader->last->type == CATEGORY_NODE_TYPE_LOGIC ||
             reader->last->type == CATEGORY_NODE_TYPE_ALL ||
             reader->last->type == CATEGORY_NODE_TYPE_AND ||
             reader->last->type == CATEGORY_NODE_TYPE_OR ||
             reader->last->type == CATEGORY_NODE_TYPE_NOT)
    {
        start_logic_child_element(reader, context, element_name, attribute_names, attribute_values, error);
    }
    else if (reader->last->type == CATEGORY_NODE_TYPE_INCLUDE ||
             reader->last->type == CATEGORY_NODE_TYPE_EXCLUDE)
    {
        start_include_exclude_element(reader, context, element_name, attribute_names, attribute_values, error);
    }
    else
    {
        set_error(error, context,
                  G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                  "Element <%s> may not appear in this context\n",
                  element_name);
    }
}

static void end_element_handler(GMarkupParseContext *context,
                                const char *element_name,
                                gpointer user_data,
                                GError **error)
{
    KiranCategoryReader *reader = KIRAN_CATEGORY_READER(user_data);

    g_assert(reader->last != NULL);

    switch (reader->last->type)
    {
        case CATEGORY_NODE_TYPE_NAME:
        case CATEGORY_NODE_TYPE_ICON:
        case CATEGORY_NODE_TYPE_REPEAT:
        case CATEGORY_NODE_TYPE_DESKTOP_ID:
        case CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
            if (reader->last->content == NULL)
            {
                set_error(error, context,
                          G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                          "Element <%s> is required to contain text and was empty\n",
                          element_name);
            }
            break;
        case CATEGORY_NODE_TYPE_CATEGORY:
            if (!has_child_of_type(reader->last, CATEGORY_NODE_TYPE_NAME))
            {
                set_error(error, context,
                          G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                          "<category> elements are required to contain a <name> element\n");
            }
            break;
        default:
            break;
    }

    reader->last = reader->last->parent;
}

static gboolean all_whitespace(const char *text,
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

static void text_handler(GMarkupParseContext *context,
                         const char *text,
                         gsize text_len,
                         gpointer user_data,
                         GError **error)
{
    KiranCategoryReader *reader = KIRAN_CATEGORY_READER(user_data);

    switch (reader->last->type)
    {
        case CATEGORY_NODE_TYPE_NAME:
        case CATEGORY_NODE_TYPE_ICON:
        case CATEGORY_NODE_TYPE_DESKTOP_ID:
        case CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
            reader->last->content = str_trim(text);
            break;
        case CATEGORY_NODE_TYPE_REPEAT:
            reader->last->content = str_trim(text);
            if (g_strcmp0(reader->last->content, "true") != 0 &&
                g_strcmp0(reader->last->content, "false") != 0)
            {
                set_error(error, context,
                          G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                          "the text in <repeat> must be true or false.",
                          g_markup_parse_context_get_element(context));
                g_clear_pointer(&(reader->last->content), g_free);
            }
            break;

        case CATEGORY_NODE_TYPE_ROOT:
        case CATEGORY_NODE_TYPE_CATEGORIES:
        case CATEGORY_NODE_TYPE_CATEGORY:
        case CATEGORY_NODE_TYPE_INCLUDE:
        case CATEGORY_NODE_TYPE_EXCLUDE:
        case CATEGORY_NODE_TYPE_ALL:
        case CATEGORY_NODE_TYPE_AND:
        case CATEGORY_NODE_TYPE_OR:
        case CATEGORY_NODE_TYPE_NOT:
            if (!all_whitespace(text, text_len))
            {
                set_error(error, context,
                          G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                          "No text is allowed inside element <%s>",
                          g_markup_parse_context_get_element(context));
            }
            break;
    }
}

KiranCategoryNode *kiran_category_reader_from_xml(KiranCategoryReader *self, const gchar *file_path)
{
    GMarkupParseContext *context;
    GError *error;
    char *text;
    gsize length;

    text = NULL;
    length = 0;
    context = NULL;

    g_debug("Loading \"%s\" from disk\n", file_path);

    if (!g_file_get_contents(file_path,
                             &text,
                             &length,
                             &error))
    {
        g_warning("Failed to load \"%s\"\n",
                  file_path);
        return NULL;
    }

    context = g_markup_parse_context_new(&category_funcs, 0, self, NULL);

    error = NULL;
    if (!g_markup_parse_context_parse(context,
                                      text,
                                      length,
                                      &error))
        goto out;

    error = NULL;
    g_markup_parse_context_end_parse(context, &error);

out:
    if (context)
        g_markup_parse_context_free(context);
    g_free(text);

    if (error)
    {
        g_warning("Error \"%s\" loading \"%s\"\n",
                  error->message, file_path);
    }

    return self->root;
}

static void kiran_category_reader_init(KiranCategoryReader *self)
{
    self->root = kiran_category_node_get_new(CATEGORY_NODE_TYPE_ROOT);
    self->last = self->root;
}

static void kiran_category_reader_class_init(KiranCategoryReaderClass *klass)
{
}

KiranCategoryReader *kiran_category_reader_get_new()
{
    return g_object_new(KIRAN_TYPE_CATEGORY_READER, NULL);
}