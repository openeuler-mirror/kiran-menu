#include "kirancategoryitem.h"
#include <iostream>
#include <glibmm/i18n.h>


KiranCategoryItem::KiranCategoryItem(const std::string &name,
                                     bool clickable):
    Glib::ObjectBase("KiranCategoryItem"),
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
void KiranCategoryItem::set_clickable(bool clickable)
{
    this->clickable = clickable;
    if (clickable)
        add_events(Gdk::BUTTON_PRESS_MASK);
    else {
        Gdk::EventMask events = get_events();
        //不再关注鼠标点击事件
        set_events(events &  ~Gdk::BUTTON_PRESS_MASK);
    }
    set_can_focus(clickable);
}

const std::string &KiranCategoryItem::get_category_name()
{
    return category_name;
}

sigc::signal<void> KiranCategoryItem::signal_clicked()
{
    return m_signal_clicked;
}

void KiranCategoryItem::set_category_name(const std::string &name)
{
    category_name = name;
    set_text(category_name);
}

bool KiranCategoryItem::on_button_press_event(GdkEventButton *button_event)
{
    if (!clickable || gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        //忽略鼠标右键事件
        return false;
    }

    m_signal_clicked.emit();
    return false;
}

bool KiranCategoryItem::on_key_press_event(GdkEventKey *key_event)
{
    if (!clickable)
        return true;
    if (key_event->keyval == GDK_KEY_Return)
        m_signal_clicked.emit();

    return false;
}

bool KiranCategoryItem::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return KiranMenuListItem::on_enter_notify_event(crossing_event);
}

bool KiranCategoryItem::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    if (!clickable)
        return false;

    return KiranMenuListItem::on_leave_notify_event(crossing_event);
}

