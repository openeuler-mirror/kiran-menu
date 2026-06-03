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

#include "recent-files-list-box.h"
#include <glibmm/i18n.h>
#include "kiran-helper.h"
#include "kiran-opacity-menu.h"
#include "lib/base.h"
#include "recent-file-item.h"
#include <algorithm>

RecentFilesListBox::RecentFilesListBox() : filter_pattern("*")
{
    set_activate_on_single_click(false);
    set_filter_func(sigc::mem_fun(*this, &RecentFilesListBox::on_filter));

    get_style_context()->add_class("menu-recent-list");

    /* 最近访问文件列表发生变化时重新加载 */
    Gtk::RecentManager::get_default()->signal_changed().connect(sigc::mem_fun(this, &RecentFilesListBox::load));
    load();
}

void RecentFilesListBox::set_filter_pattern(const Glib::ustring &filter_pattern_)
{
    KLOG_DEBUG("filter pattern is '%s'", filter_pattern_.c_str());
    filter_pattern = filter_pattern_;
    invalidate_filter();
}

void RecentFilesListBox::load()
{
    clean();

    std::vector<Glib::RefPtr<Gtk::RecentInfo>> items = Gtk::RecentManager::get_default()->get_items();
    std::sort(items.begin(), items.end(), [](Glib::RefPtr<Gtk::RecentInfo> a, Glib::RefPtr<Gtk::RecentInfo> b)
              { return a->get_modified() > b->get_modified(); });

    for (auto info : items)
    {
        RecentFileItem* recent_item = Gtk::make_managed<RecentFileItem>(info);
        recent_item->show_all();
        this->insert(*(Gtk::ListBoxRow*)recent_item, -1);
    }
    invalidate_filter();
}

void RecentFilesListBox::clean()
{
    /* 清空原有信息 */
    while (true)
    {
        auto row = get_row_at_index(0);
        if (!row)
            break;

        remove(*row);
        delete row;
    }
}

void RecentFilesListBox::on_row_activated(Gtk::ListBoxRow *row)
{
    auto recent_item = reinterpret_cast<RecentFileItem*>(row);
    recent_item->open_file();
}

bool RecentFilesListBox::on_filter(Gtk::ListBoxRow *row)
{
    GtkRecentFilterInfo filter_info;
    auto recent_item = reinterpret_cast<RecentFileItem*>(row);

    if ( filter_pattern == "*" || filter_pattern.empty() )
        return true;

    Glib::ustring name(recent_item->display_name());
    Glib::ustring uri(recent_item->uri());

    auto filter = Gtk::RecentFilter::create();
    filter->add_pattern(filter_pattern);
    filter_info.contains = gtk_recent_filter_get_needed(filter->gobj());
    filter_info.display_name = name.c_str();
    
    gboolean res = gtk_recent_filter_filter(filter->gobj(), &filter_info);
    return res;
}
