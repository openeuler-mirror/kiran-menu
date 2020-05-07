/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 16:47:11
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-writer.c
 */

#include "lib/kiran-category-writer.h"

struct _KiranCategoryWriter
{
    GObject parent_instance;
};

G_DEFINE_TYPE(KiranCategoryWriter, kiran_category_writer, G_TYPE_OBJECT);

#define PUT_STR_TO_CATEGORY_FILE(str)                                    \
    if (!g_data_output_stream_put_string(data_output, str, NULL, error)) \
    {                                                                    \
        return FALSE;                                                    \
    }

static gboolean write_prefix_space(gint recurse_level, GDataOutputStream *data_output, GError **error)
{
    for (gint i = 0; i < recurse_level; ++i)
    {
        PUT_STR_TO_CATEGORY_FILE("    ");
    }
    return TRUE;
}

#define PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, str)                \
    {                                                                        \
        if (!write_prefix_space(recurse_level, data_output, error))          \
        {                                                                    \
            return FALSE;                                                    \
        }                                                                    \
        if (!g_data_output_stream_put_string(data_output, str, NULL, error)) \
        {                                                                    \
            return FALSE;                                                    \
        }                                                                    \
    }

static gboolean write_rule(KiranCategoryNode *node, gint recurse_level, GDataOutputStream *data_output, GError **error)
{
    g_return_val_if_fail(node != NULL, FALSE);

    KiranCategoryNode *iter = node->children;

    for (; iter; iter = (iter->next == node->children) ? NULL : iter->next)
    {
        switch (iter->type)
        {
            case CATEGORY_NODE_TYPE_AND:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<and>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</and>\n");
                break;
            case CATEGORY_NODE_TYPE_OR:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<or>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</or>\n");
                break;
            case CATEGORY_NODE_TYPE_NOT:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<not>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</not>\n");
                break;
            case CATEGORY_NODE_TYPE_DESKTOP_ID:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<desktop_id> ");
                PUT_STR_TO_CATEGORY_FILE(iter->content);
                PUT_STR_TO_CATEGORY_FILE(" </desktop_id>\n");
                break;
            case CATEGORY_NODE_TYPE_DESKTOP_CATEGORY:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<desktop_category> ");
                PUT_STR_TO_CATEGORY_FILE(iter->content);
                PUT_STR_TO_CATEGORY_FILE(" </desktop_category>\n");
                break;
            default:
                break;
        }
    }
    return TRUE;
}

static gboolean write_category(KiranCategoryNode *node, gint recurse_level, GDataOutputStream *data_output, GError **error)
{
    g_return_val_if_fail(node != NULL, FALSE);
    g_return_val_if_fail(node->type == CATEGORY_NODE_TYPE_CATEGORY, FALSE);

    PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, "<category>\n");

    KiranCategoryNode *iter = node->children;

    for (; iter; iter = (iter->next == node->children) ? NULL : iter->next)
    {
        switch (iter->type)
        {
            case CATEGORY_NODE_TYPE_NAME:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<name> ");
                PUT_STR_TO_CATEGORY_FILE(iter->content);
                PUT_STR_TO_CATEGORY_FILE(" </name>\n");
                break;
            case CATEGORY_NODE_TYPE_ICON:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<icon> ");
                PUT_STR_TO_CATEGORY_FILE(iter->content);
                PUT_STR_TO_CATEGORY_FILE(" </icon>\n");
                break;
            case CATEGORY_NODE_TYPE_LOGIC:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<logic>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</logic>\n");
                break;
            case CATEGORY_NODE_TYPE_INCLUDE:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<include>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</include>\n");
                break;
            case CATEGORY_NODE_TYPE_EXCLUDE:
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "<exclude>\n");
                write_rule(iter, recurse_level + 1, data_output, error);
                PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level + 1, "</exclude>\n");
                break;
            default:
                break;
        }
    }

    PUT_STR_WITH_LEV_TO_CATEGORY_FILE(recurse_level, "</category>\n");
}

static gboolean write_root(KiranCategoryNode *node, GDataOutputStream *data_output, GError **error)
{
    PUT_STR_TO_CATEGORY_FILE("<?xml version=\"1.0\"?>\n\n<categories>");

    KiranCategoryNode *categories = node->children;

    if (categories)
    {
        KiranCategoryNode *category = categories->children;
        for (; category; category = (category->next == categories->children) ? NULL : category->next)
        {
            if (!write_category(category, 1, data_output, error))
            {
                return FALSE;
            }
        }
    }

    PUT_STR_TO_CATEGORY_FILE("</categories>\n");

    return TRUE;
}

gboolean kiran_category_writer_to_xml(KiranCategoryWriter *writer, KiranCategoryNode *node, const gchar *file_path)
{
    GFileOutputStream *output = NULL;
    GOutputStream *buffered_output = NULL;
    GDataOutputStream *data_output = NULL;
    GError *error = NULL;

    g_autoptr(GFile) config_file = NULL;

    config_file = g_file_new_for_path(file_path);

    if (!config_file)
    {
        g_warning("file %s create fail.\n", file_path);
        return FALSE;
    }

    output = g_file_replace(config_file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error);
    if (!output)
    {
        g_warning("Could not save menu category data: %s", error->message);
        g_error_free(error);
        return FALSE;
    }

    buffered_output = g_buffered_output_stream_new(G_OUTPUT_STREAM(output));
    g_object_unref(output);

    data_output = g_data_output_stream_new(G_OUTPUT_STREAM(buffered_output));
    g_object_unref(buffered_output);

    write_root(node, data_output, &error);

    if (!error)
        g_output_stream_close_async(G_OUTPUT_STREAM(data_output), 0, NULL, NULL, NULL);
    g_object_unref(data_output);

    if (error)
    {
        g_debug("Could not save menu category data: %s", error->message);
        g_error_free(error);
        return FALSE;
    }
    return TRUE;
}

static void kiran_category_writer_init(KiranCategoryWriter *self)
{
}

static void kiran_category_writer_class_init(KiranCategoryWriterClass *klass)
{
}

KiranCategoryWriter *kiran_category_writer_get_new()
{
    return g_object_new(KIRAN_TYPE_CATEGORY_WRITER, NULL);
}