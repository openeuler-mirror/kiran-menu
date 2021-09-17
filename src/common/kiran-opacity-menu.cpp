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
