/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-07 11:13:55
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-writer.h
 */

#pragma once

#include <gio/gio.h>

#include "lib/kiran-category-node.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_CATEGORY_WRITER (kiran_category_writer_get_type())

G_DECLARE_FINAL_TYPE(KiranCategoryWriter, kiran_category_writer, KIRAN,
                     CATEGORY_WRITER, GObject);

KiranCategoryWriter* kiran_category_writer_get_new();

gboolean kiran_category_writer_to_xml(KiranCategoryWriter* writer, KiranCategoryNode* node, const gchar* file_path);