/*
 * @Author       : tangjie02
 * @Date         : 2020-04-30 17:28:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-06 10:51:40
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/kiran-category-reader.h
 */

#pragma once

#include <gio/gio.h>

#include "lib/kiran-category-node.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_CATEGORY_READER (kiran_category_reader_get_type())

G_DECLARE_FINAL_TYPE(KiranCategoryReader, kiran_category_reader, KIRAN,
                     CATEGORY_READER, GObject);

KiranCategoryReader* kiran_category_reader_get_new();

KiranCategoryNode* kiran_category_reader_from_xml(KiranCategoryReader* reader, const gchar* file_path);
