#include "menu-category-item.h"
#include <iostream>
#include <glibmm/i18n.h>


MenuCategoryItem::MenuCategoryItem(const std::string &name,
                                     bool clickable):
    Glib::ObjectBase("KiranMenuCategoryItem"),
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
    if (!clickable || gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
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
