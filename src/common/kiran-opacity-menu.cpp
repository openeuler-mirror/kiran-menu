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

#include "kiran-opacity-menu.h"

KiranOpacityMenu::KiranOpacityMenu()
{
    get_style_context()->add_class("opacity-menu");
}

bool KiranOpacityMenu::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto window = get_window();

    cr->save();

    //先将窗口背景设置透明，然后再绘制背景和边框，否则圆角周围会出现多余的窗口背景色
    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->paint();
    cr->set_operator(Cairo::OPERATOR_SOURCE);
    get_style_context()->render_background(cr, 0, 0,
                                           window->get_width(),
                                           window->get_height());
    get_style_context()->render_frame(cr, 0, 0,
                                      window->get_width(),
                                      window->get_height());
    cr->restore();

    for (auto child : get_children())
    {
        propagate_draw(*child, cr);
    }

    return false;
}
