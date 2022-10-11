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

#ifndef SHOWDESKTOPAPPLETBUTTON_H
#define SHOWDESKTOPAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class ShowDesktopAppletButton : public Gtk::ToggleButton
{
public:
    ShowDesktopAppletButton(MatePanelApplet *applet_);

protected:
    virtual void on_map() override;
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_show_desktop_changed(bool show);
    virtual void on_toggled();

private:
    MatePanelApplet *applet;

    Gtk::Orientation get_applet_orientation() const;
};

#endif  // SHOWDESKTOPAPPLETBUTTON_H