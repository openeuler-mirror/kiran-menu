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
