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

#include "window-thumbnail-widget.h"
#include "kiran-helper.h"

WindowThumbnailWidget::WindowThumbnailWidget(KiranWindowPointer &window_) : window(window_)
{
    window_->signal_name_changed().connect(
        sigc::mem_fun(*this, &WindowThumbnailWidget::update_title));

    update_title();

    get_style_context()->add_class("window-thumbnail");
}

KiranWindowPointer WindowThumbnailWidget::get_window_() const
{
    return window.lock();
}

void WindowThumbnailWidget::on_close_button_clicked()
{
    auto window = get_window_();
    if (!window)
        return;
    window->close();
}

void WindowThumbnailWidget::update_title()
{
    if (window.expired())
        return;
    set_title(window.lock()->get_name());
}

bool WindowThumbnailWidget::draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    int scale_factor;
    GdkPixbuf *pixbuf = nullptr;

    if (window.expired())
        return false;

    scale_factor = get_scale_factor();
    pixbuf = window.lock()->get_icon();
    auto icon = Glib::wrap(pixbuf, true)->scale_simple(24 * scale_factor, 24 * scale_factor, Gdk::INTERP_BILINEAR);

    cr->scale(1.0 / scale_factor, 1.0 / scale_factor);
    Gdk::Cairo::set_source_pixbuf(cr, icon, 0, 0);
    cr->paint();
    return false;
}

bool WindowThumbnailWidget::draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    return false;
}
