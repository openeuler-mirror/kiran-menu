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

#ifndef MENU_APP_ITEM_H
#define MENU_APP_ITEM_H

#include "kiran-opacity-menu.h"
#include "menu-list-item-widget.h"
#include "menu-skeleton.h"

class MenuAppItem : public MenuListItemWidget
{
public:
    MenuAppItem(const std::shared_ptr<Kiran::App> &_app, int icon_size = 24, Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);
    ~MenuAppItem() = default;
    sigc::signal<void> signal_launched();
    virtual void set_orientation(Gtk::Orientation orient) override;
    void launch_app();

    const std::shared_ptr<Kiran::App> get_app() const;

protected:
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual void on_clicked() override;
    virtual void on_drag_begin(const Glib::RefPtr<Gdk::DragContext> &context) override;
    virtual void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::SelectionData &selection, guint info, guint timestamp) override;
    virtual void on_drag_end(const Glib::RefPtr<Gdk::DragContext> &context) override;

    virtual void on_context_menu_deactivated();

    virtual void init_drag_and_drop();

    bool pin_app_to_taskbar();
    bool unpin_app_from_taskbar();

    void create_context_menu();
    bool add_app_to_desktop();

private:
    KiranOpacityMenu context_menu;
    Gtk::MenuItem *items;
    std::weak_ptr<Kiran::App> app;

    bool menu_shown;
    sigc::signal<void> m_signal_launched;

    bool is_in_favorite();
    bool is_fixed_on_taskbar();
};

#endif  // MENU_APP_ITEM_H
