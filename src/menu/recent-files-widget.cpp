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

#include "recent-files-widget.h"
#include <glibmm/i18n.h>

RecentFilesWidget::RecentFilesWidget() : box(Gtk::ORIENTATION_VERTICAL, 0)
{
    init_ui();
    auto manager = Gtk::RecentManager::get_default();

    manager->signal_changed().connect(
        sigc::mem_fun(*this, &RecentFilesWidget::on_recent_list_changed));
}

void RecentFilesWidget::init_ui()
{
    empty_label.set_text(_("The Recently files list is empty!"));
    empty_label.get_style_context()->add_class("empty-prompt-text");
    scrolled.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scrolled.set_hexpand(true);
    scrolled.set_vexpand(true);
    scrolled.set_halign(Gtk::ALIGN_FILL);
    scrolled.set_valign(Gtk::ALIGN_FILL);

    box.add(search_entry);
    box.add(scrolled);

    on_recent_list_changed();
    box.show_all();
    add(box);

    search_entry.set_placeholder_text(_("Search files"));
    search_entry.get_style_context()->add_class("menu-search-entry");
    search_entry.signal_changed().connect(
        sigc::mem_fun(*this, &RecentFilesWidget::on_search_changed));
}

void RecentFilesWidget::on_recent_list_changed()
{
    auto manager = Gtk::RecentManager::get_default();
    if (manager->get_items().size() != 0)
    {
        if (list.get_parent() == nullptr)
        {
            if (scrolled.get_child() != nullptr)
            {
                scrolled.remove();
            }
            scrolled.add(list);
        }
    }
    else
    {
        if (empty_label.get_parent() == nullptr)
        {
            if (scrolled.get_child() != nullptr)
            {
                scrolled.remove();
            }
            scrolled.add(empty_label);
        }
    }

    scrolled.show_all();
}

void RecentFilesWidget::on_search_changed()
{
    auto text = search_entry.get_text();

    list.set_filter_pattern(Glib::ustring::compose("*%1*", text));
}