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

#include "menu-category-item.h"
#include <glibmm/i18n.h>
#include <iostream>

// TODO: 菜单中的应用分类后端目前没有做翻译，暂时放到前端调用_("xxx")进行翻译，正常应该是后端翻译后返回给前端，后续需要调整这里的翻译逻辑
#define MENU_CATEGORY_OFFICE N_("Office")
#define MENU_CATEGORY_DEVELOPMENT N_("Development")
#define MENU_CATEGORY_NETWORK N_("Network")
#define MENU_CATEGORY_GRAPHICS N_("Graphics")
#define MENU_CATEGORY_MULTIMEDIA N_("Multimedia")
#define MENU_CATEGORY_UTILITIES N_("Utilities")
#define MENU_CATEGORY_SETTINGS N_("Settings")
#define MENU_CATEGORY_OTHERS N_("Others")

MenuCategoryItem::MenuCategoryItem(const std::string &name,
                                   bool clickable) : Glib::ObjectBase("KiranMenuCategoryItem"),
                                                     MenuListItemWidget("kiran-menu-group-symbolic", _(name.c_str()))
{
    auto context = get_style_context();

    context->add_class("menu-category-item");
    context->add_class("flat");
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
