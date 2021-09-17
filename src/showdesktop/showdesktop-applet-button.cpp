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