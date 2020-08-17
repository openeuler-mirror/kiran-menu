#ifndef KIRANAPPITEM_H
#define KIRANAPPITEM_H

#include "kiran-menu-list-item.h"
#include "menu-skeleton.h"
#include "kiran-opacity-menu.h"

class KiranMenuAppItem : public KiranMenuListItem
{
public:
    KiranMenuAppItem(const std::shared_ptr<Kiran::App> &_app, int icon_size = 24, Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);
    ~KiranMenuAppItem() = default;
    sigc::signal<void> signal_launched();
    virtual void set_orientation(Gtk::Orientation orient) override;
    void launch_app();

    const std::shared_ptr<Kiran::App> get_app() const;

protected:
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual void on_clicked() override;
    virtual void init_drag_and_drop();

    void create_context_menu();
    bool add_app_to_desktop();

private:
    KiranOpacityMenu context_menu;
    Gtk::MenuItem *items;
    std::weak_ptr<Kiran::App> app;

    bool menu_shown;
    sigc::signal <void> m_signal_launched;

    bool is_in_favorite();

};

#endif // KIRANAPPITEM_H
