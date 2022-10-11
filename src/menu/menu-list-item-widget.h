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

#ifndef MENU_LIST_ITEM_WIDGET_H
#define MENU_LIST_ITEM_WIDGET_H

#include <gtkmm.h>

class MenuListItemWidget : public Gtk::Button
{
public:
    MenuListItemWidget(int icon_size,
                       Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);

    MenuListItemWidget(const Glib::RefPtr<Gio::Icon> &gicon,
                       const std::string &text,
                       Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                       int icon_size = 24);

    MenuListItemWidget(const std::string &resource,
                       const std::string &text,
                       Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL,
                       int icon_size = 24);

    void set_text(const std::string &text);
    void set_icon(const Glib::RefPtr<Gio::Icon> &gicon, int icon_size = 24);
    void set_icon(const std::string &resource, int icon_size = 24);
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
