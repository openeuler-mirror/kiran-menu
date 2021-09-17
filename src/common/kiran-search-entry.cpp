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
