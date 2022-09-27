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

#include "menu-avatar-widget.h"
#include <iostream>
#include "../config.h"
#include "kiran-helper.h"
#include "lib/base.h"

MenuAvatarWidget::MenuAvatarWidget(int size) : icon_size(size),
                                               user_info(getuid())
{
    set_size_request(size, size);
    add_events(Gdk::BUTTON_PRESS_MASK);
    set_halign(Gtk::ALIGN_CENTER);
    set_valign(Gtk::ALIGN_CENTER);

    get_style_context()->add_class("circular");

    user_info.signal_changed().connect(
        sigc::mem_fun(*this, &MenuAvatarWidget::load_user_info));
    load_user_info();
}

void MenuAvatarWidget::load_user_info_()
{
    if (!user_info.is_ready())
    {
        /* 用户信息尚未加载完全，延后加载 */
        KLOG_DEBUG("data is not ready, request to load");
        user_info.signal_ready().connect(
            sigc::mem_fun(*this, &MenuAvatarWidget::load_user_info));
        user_info.load();
        return;
    }

    if (user_info.get_username() != NULL)
    {
        set_tooltip_text(user_info.get_username());
    }
    queue_draw();
}

void MenuAvatarWidget::load_user_info()
{
    Glib::signal_idle().connect_once(
        sigc::mem_fun(*this, &MenuAvatarWidget::load_user_info_));
}

void MenuAvatarWidget::set_icon_size(int size_)
{
    if (icon_size == size_)
        return;
    icon_size = size_;
    set_size_request(icon_size, icon_size);
    queue_resize();
}

bool MenuAvatarWidget::on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr)
{
    int scale;
    double radius;
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    if (!user_info.is_ready())
    {
        return false;
    }

    allocation = get_allocation();
    scale = get_scale_factor();
    radius = allocation.get_width() > allocation.get_height() ? allocation.get_height() : allocation.get_width();
    radius /= 2.0;
    try
    {
        auto icon_file = user_info.get_iconfile();
        pixbuf = Gdk::Pixbuf::create_from_file(icon_file ? icon_file : std::string(),
                                               allocation.get_width() * scale,
                                               allocation.get_height() * scale);
    }
    catch (const Glib::Error &e)
    {
        /* 加载默认头像 */
        KLOG_WARNING("Failed to load avatar: %s", e.what().c_str());
        pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-menu/icon/avatar",
                                                   allocation.get_width() * scale,
                                                   allocation.get_height() * scale);
    }

    cr->save();

    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->clip();

    cr->scale(1.0 / scale, 1.0 / scale);
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->paint();

    /* 绘制头像边框 */
    cr->scale(1.0 * scale, 1.0 * scale);
    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->set_source_rgba(1.0, 1.0, 1.0, 0.2);
    cr->set_line_width(1);
    cr->stroke();

    cr->restore();
    return false;
}

void MenuAvatarWidget::on_clicked()
{
    const char *app_names[] = {
#ifdef BUILD_WITH_KIRANACCOUNTS
        "kiran-account-manager",
        "kiran-cpanel-account",
#else
        "mate-about-me",
#endif
        "system-config-users",
        nullptr};

    if (!KiranHelper::launch_app_from_list(app_names))
        KLOG_WARNING("Failed to launch avatar or account manage tools");
}
