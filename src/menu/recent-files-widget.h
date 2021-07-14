/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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