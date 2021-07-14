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

#include "kiran-search-entry.h"
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <iostream>

void on_preedit_text_changed(KiranSearchEntry *entry, char *preedit_text)
{
    entry->has_preedit_text = (strlen(preedit_text) != 0);
}

KiranSearchEntry::KiranSearchEntry() : has_preedit_text(false)
{
    auto context = get_style_context();

    auto pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-menu/action/search");

    set_icon_from_pixbuf(pixbuf);
    set_placeholder_text(_("Search for applications"));
    set_halign(Gtk::ALIGN_FILL);
    set_valign(Gtk::ALIGN_CENTER);

    context->add_class("kiran-search-entry");

    g_signal_connect_swapped(gobj(), "preedit-changed", G_CALLBACK(on_preedit_text_changed), this);
}

bool KiranSearchEntry::on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gdk::RGBA color;
    auto context = get_style_context();

    Gtk::SearchEntry::on_draw(cr);

    if (!is_focus() || get_text_length() > 0 || has_preedit_text)
        return false;

    /**
     * 搜索框已获取焦点，无输入内容和预编辑文本，需要绘制占位文本
     */
    int offset_x, offset_y;
    auto layout = get_layout();

    layout->set_text(get_placeholder_text());
    get_layout_offsets(offset_x, offset_y);

    if (context->lookup_color("placeholder_text_color", color))
        Gdk::Cairo::set_source_rgba(cr, color);
    else
        cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->move_to(offset_x, offset_y);

    layout->show_in_cairo_context(cr);
    layout->set_text("");
    return false;
}
