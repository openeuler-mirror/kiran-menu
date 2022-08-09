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

#ifndef MENU_APP_ITEM_H
#define MENU_APP_ITEM_H

#include "kiran-opacity-menu.h"
#include "menu-list-item-widget.h"
#include "menu-skeleton.h"

class MenuAppItem : public MenuListItemWidget
{
public:
    MenuAppItem(const std::shared_ptr<Kiran::App> &_app, int icon_size = 24, Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);
    virtual ~MenuAppItem();
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
    bool on_drag_failed(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::DragResult result);

    virtual void init_drag_and_drop();

    bool pin_app_to_taskbar();
    bool unpin_app_from_taskbar();

    void create_context_menu();
    bool add_app_to_desktop();

private:
    void on_add_favorite_app();
    void on_del_favorite_app();

private:
    KiranOpacityMenu context_menu;
    Gtk::MenuItem *items;
    std::weak_ptr<Kiran::App> app;

    bool menu_shown;
    sigc::signal<void> m_signal_launched;
    sigc::connection idle_drag_connection_;

    bool is_in_favorite();
    bool is_fixed_on_taskbar();
};

#endif  // MENU_APP_ITEM_H
