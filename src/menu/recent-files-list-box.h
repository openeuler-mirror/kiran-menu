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

#ifndef KIRAN_RECENTLY_WIDGET_INCLUDE_H
#define KIRAN_RECENTLY_WIDGET_INCLUDE_H

#include <gtkmm.h>

class RecentFilesListBox : public Gtk::ListBox
{
public:
    RecentFilesListBox();
    void set_filter_pattern(const Glib::ustring &filter_pattern);

protected:
    virtual void on_row_activated(Gtk::ListBoxRow *row) override;

private:
    void load();
    void clean();
    bool on_filter(Gtk::ListBoxRow *row);

    Gtk::Widget *create_recent_item(const Glib::RefPtr<Gtk::RecentInfo> &info);
    Gtk::Menu *create_context_menu(const Glib::RefPtr<Gtk::RecentInfo> &info);

private:
    Glib::ustring filter_pattern;
};

#endif