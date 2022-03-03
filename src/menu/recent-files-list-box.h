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
    void load();

    bool on_filter(Gtk::ListBoxRow *row);

    Gtk::Widget *create_recent_item(const Glib::RefPtr<Gtk::RecentInfo> &info);
    Gtk::Menu *create_context_menu(const Glib::RefPtr<Gtk::RecentInfo> &info);

private:
    Glib::ustring filter_pattern;
};

#endif