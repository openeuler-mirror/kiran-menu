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

#include "showdesktop-applet-button.h"
#include <screen-manager.h>

ShowDesktopAppletButton::ShowDesktopAppletButton(MatePanelApplet *applet_) : applet(applet_)
{
    Kiran::ScreenManager::get_instance()->signal_show_desktop_changed().connect(
        sigc::mem_fun(*this, &ShowDesktopAppletButton::on_show_desktop_changed));

    property_active().signal_changed().connect(
        sigc::mem_fun(*this, &ShowDesktopAppletButton::on_toggled));

    get_style_context()->add_class("kiran-showdesktop-button");
}

void ShowDesktopAppletButton::on_toggled()
{
    bool active = get_active();
    Kiran::ScreenManager *manager = Kiran::ScreenManager::get_instance();

    if (manager->get_show_desktop() != active)
        manager->set_show_desktop(active);
}

Gtk::Orientation ShowDesktopAppletButton::get_applet_orientation() const
{
    switch (mate_panel_applet_get_orient(applet))
    {
    case MATE_PANEL_APPLET_ORIENT_UP:
    case MATE_PANEL_APPLET_ORIENT_DOWN:
        return Gtk::ORIENTATION_HORIZONTAL;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        return Gtk::ORIENTATION_VERTICAL;
    default:
        g_return_val_if_reached(Gtk::ORIENTATION_HORIZONTAL);
    }
}

void ShowDesktopAppletButton::on_map()
{
    Gtk::ToggleButton::on_map();

    set_active(Kiran::ScreenManager::get_instance()->get_show_desktop());
}

void ShowDesktopAppletButton::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{
    if (get_applet_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        min_width = natural_width = 20;
    else
        min_width = natural_width = mate_panel_applet_get_size(applet);
}

void ShowDesktopAppletButton::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{
    if (get_applet_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        min_height = natural_height = mate_panel_applet_get_size(applet);
    else
        min_height = natural_height = 20;
}

void ShowDesktopAppletButton::on_show_desktop_changed(bool show)
{
    if (get_active() == show)
        return;

    set_active(show);
}