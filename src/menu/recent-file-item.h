/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-menu is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <gtkmm.h>

class RecentFileItem : public Gtk::ListBoxRow
{
public:
    RecentFileItem(const Glib::RefPtr<Gtk::RecentInfo>& info);
    ~RecentFileItem();

    Glib::ustring uri() const;
    Glib::ustring display_name() const;

    void open_file();

private:
    void init();
    void init_context_menu(Gtk::Widget* widget);
    
private:
    bool check_exists();
    bool item_pressed(const GdkEventButton *button_event);
    void open_file_lcation();
    void remove_file_from_list();
    void clear_files_list();

private:
    Glib::RefPtr<Gtk::RecentInfo> info_;
    Glib::RefPtr<Gtk::Menu> menu_;
};