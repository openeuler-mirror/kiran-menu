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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#ifndef __RECENT_FILES_WIDGET_H__
#define __RECENT_FILES_WIDGET_H__

#include <gtkmm.h>
#include "kiran-search-entry.h"
#include "recent-files-list-box.h"

// 最近访问文档列表页面
class RecentFilesWidget : public Gtk::Box
{
public:
    RecentFilesWidget();

protected:
    void init_ui();
    void on_recent_list_changed();
    void on_search_changed();

private:
    Gtk::Box box;
    KiranSearchEntry search_entry;
    Gtk::ScrolledWindow scrolled;
    RecentFilesListBox list;
    Gtk::Label empty_label;
};

#endif  // __RECENT_FILES_WIDGET_H__