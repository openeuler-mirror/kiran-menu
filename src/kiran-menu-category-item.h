#ifndef KIRANCATEGORYITEM_H
#define KIRANCATEGORYITEM_H

#include "kiran-menu-list-item.h"

class KiranMenuCategoryItem : public KiranMenuListItem
{
public:
    KiranMenuCategoryItem(const std::string &name, bool clickable);
    const std::string &get_category_name();
    sigc::signal<void> signal_clicked();

protected:
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;

    void set_category_name(const std::string &name);
    void set_clickable(bool clickable);

private:
    gboolean clickable;
    std::string category_name;

    sigc::signal<void> m_signal_clicked;
    const static std::string icon_resource;
};

#endif // KIRANCATEGORYITEM_H
