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

#ifndef KIRAN_RECENTLY_WIDGET_INCLUDE_H
#define KIRAN_RECENTLY_WIDGET_INCLUDE_H

#include <gtkmm.h>

class RecentFilesListBox: public Gtk::ListBox
{
public:
    RecentFilesListBox();
    void set_filter_pattern(const Glib::ustring &filter_pattern);
    void load();

protected:
    virtual void on_row_activated(Gtk::ListBoxRow *row) override;

    /**
     * @brief 打开info指向的文件，具体打开的方式采用系统默认方式
     * @param info 要打开的最近文档
     */
    static void open_file(const Glib::RefPtr<Gtk::RecentInfo> &info);

    /**
     * @brief 打开info指向的文件所在的目录
     * @param info 要打开所在目录的最近文档
     */
    static void open_file_location(const Glib::RefPtr<Gtk::RecentInfo> &info);

    /**
     * @brief 从最近访问的文档列表中删除info对应的文档
     * @param info 要删除的最近文档
     */
    static void remove_file_from_list(const Glib::RefPtr<Gtk::RecentInfo> &info);

    /**
     * @brief 清空最近访问的文档列表，所有的最近文档信息都会丢失
     */
    static void clear_files_list(void);

private:
    bool on_filter(Gtk::ListBoxRow* row);

    Gtk::Widget *create_recent_item(const Glib::RefPtr<Gtk::RecentInfo> &info);
    Gtk::Menu *create_context_menu(const Glib::RefPtr<Gtk::RecentInfo> &info);

private:
    Glib::ustring filter_pattern;
};

#endif