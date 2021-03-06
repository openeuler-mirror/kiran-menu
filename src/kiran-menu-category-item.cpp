#include "kiran-menu-category-item.h"
#include <iostream>
#include <glibmm/i18n.h>


KiranMenuCategoryItem::KiranMenuCategoryItem(const std::string &name,
                                     bool clickable):
    Glib::ObjectBase("KiranMenuCategoryItem"),
    KiranMenuListItem("/kiran-menu/icon/group", _(name.c_str()))
{
    auto context = get_style_context();

    context->add_class("kiran-category-item");
    set_clickable(clickable);
    category_name = name;
}

/**
 * @brief 设置分类标签是否可点击
 * @param clickable 是否可点击
 */
void KiranMenuCategoryItem::set_clickable(bool clickable)
{
    this->clickable = clickable;
    set_can_focus(true);
}

const std::string &KiranMenuCategoryItem::get_category_name()
{
    return category_name;
}

void KiranMenuCategoryItem::set_category_name(const std::string &name)
{
    category_name = name;
    set_text(category_name);
}

bool KiranMenuCategoryItem::on_button_press_event(GdkEventButton *button_event)
{
    if (!clickable || gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        //忽略鼠标右键事件
        return false;
    }

    return KiranMenuListItem::on_button_press_event(button_event);
}

bool KiranMenuCategoryItem::on_key_press_event(GdkEventKey *key_event)
{
    if (!clickable)
        return false;
    return KiranMenuListItem::on_key_press_event(key_event);
}

bool KiranMenuCategoryItem::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return KiranMenuListItem::on_enter_notify_event(crossing_event);
}

bool KiranMenuCategoryItem::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return KiranMenuListItem::on_leave_notify_event(crossing_event);
}
