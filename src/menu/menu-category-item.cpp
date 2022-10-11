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

#include "menu-category-item.h"
#include <glibmm/i18n.h>
#include <iostream>

MenuCategoryItem::MenuCategoryItem(const std::string &name,
                                   bool clickable) : Glib::ObjectBase("KiranMenuCategoryItem"),
                                                     MenuListItemWidget("/kiran-menu/icon/group", _(name.c_str()))
{
    auto context = get_style_context();

    context->add_class("menu-category-item");
    set_clickable(clickable);
    category_name = name;
}

/**
 * @brief 设置分类标签是否可点击
 * @param clickable 是否可点击
 */
void MenuCategoryItem::set_clickable(bool clickable_)
{
    clickable = clickable_;
    set_can_focus(true);
}

const std::string &MenuCategoryItem::get_category_name() const
{
    return category_name;
}

bool MenuCategoryItem::get_clickable() const
{
    return clickable;
}

void MenuCategoryItem::set_category_name(const std::string &name)
{
    category_name = name;
    set_text(category_name);
}

bool MenuCategoryItem::on_button_press_event(GdkEventButton *button_event)
{
    if (!clickable || gdk_event_triggers_context_menu((GdkEvent *)button_event))
    {
        //忽略鼠标右键事件
        return false;
    }

    return MenuListItemWidget::on_button_press_event(button_event);
}

bool MenuCategoryItem::on_key_press_event(GdkEventKey *key_event)
{
    if (!clickable)
        return false;
    return MenuListItemWidget::on_key_press_event(key_event);
}

bool MenuCategoryItem::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return MenuListItemWidget::on_enter_notify_event(crossing_event);
}

bool MenuCategoryItem::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return MenuListItemWidget::on_leave_notify_event(crossing_event);
}
