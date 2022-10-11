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

#ifndef KIRANAPPLETBUTTON_H
#define KIRANAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class KiranAppletButton : public Gtk::ToggleButton
{
public:
    KiranAppletButton(MatePanelApplet *applet_);
    ~KiranAppletButton() override;

    MatePanelApplet *get_applet();

protected:
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void set_icon_from_resource(const std::string &resource);
    void generate_pixbuf();

private:
    MatePanelApplet *applet;

    std::string icon_resource;
    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;  //图标
    int icon_size;                          //图标显示尺寸
};

#endif  // KIRANAPPLETBUTTON_H
