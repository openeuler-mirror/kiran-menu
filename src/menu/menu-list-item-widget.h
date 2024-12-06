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

#ifndef MENU_LIST_ITEM_WIDGET_H
#define MENU_LIST_ITEM_WIDGET_H

#include <gtkmm.h>

class MenuListItemWidget : public Gtk::Button
{
public:
    explicit MenuListItemWidget(int icon_size,
                                Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);

    MenuListItemWidget(const Glib::RefPtr<Gio::Icon> &gicon,
                       const std::string &text,
                       Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                       int icon_size = 24);

    MenuListItemWidget(const std::string &icon_name,
                       const std::string &text,
                       Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                       Gtk::IconSize icon_size = Gtk::ICON_SIZE_BUTTON);

    void set_text(const std::string &text);
    void set_icon(const Glib::RefPtr<Gio::Icon> &gicon, int icon_size = 24);
    void set_icon_size(int icon_size);
    virtual void set_orientation(Gtk::Orientation orient);

    void init_ui();

protected:
    virtual void on_style_updated() override;
    virtual bool on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr) override;

    void on_orient_changed();

    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;

private:
    Gtk::Label label;
    Gtk::Image image;
    Gtk::Box box;

    Gtk::StyleProperty<int> space_property;
    Gtk::StyleProperty<int> max_width_property;
    Gtk::StyleProperty<int> max_height_property;
};

#endif  // MENU_LIST_ITEM_WIDGET_H
