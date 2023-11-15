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

#include "menu-applet-button.h"
#include <glibmm/i18n.h>
#include "kiran-helper.h"
#include "lib/base.h"

#define BUTTON_MARGIN 6

MenuAppletButton::MenuAppletButton(MatePanelApplet *panel_applet) : KiranAppletButton(panel_applet)
{
    set_tooltip_text(_("Kiran Start Menu"));
    set_icon_from_resource("/kiran-menu/icon/logo");
    get_style_context()->add_class("menu-applet-button");

    window.signal_size_changed().connect(
        sigc::mem_fun(*this, &MenuAppletButton::reposition_applet_window));

    window.signal_map().connect(
        sigc::mem_fun(*this, &MenuAppletButton::reposition_applet_window));

    window.signal_unmap_event().connect(
        [this](GdkEventAny *event) -> bool
        {
            set_active(false);
            return false;
        });
}

void MenuAppletButton::on_toggled()
{
    if (this->get_active())
    {
        // This may mean raising the window in the stacking order, deiconifying it, moving it to the current desktop,
        // and/or giving it the keyboard focus, possibly dependent on the user’s platform, window manager, and preferences.
        window.present();
    }
    else
    {
        window.hide();
    }
}

void MenuAppletButton::reposition_applet_window()
{
    MatePanelAppletOrient orient;
    int root_x, root_y;
    int window_width, window_height;
    Gtk::Allocation button_allocation;

    if (!window.get_visible())
        return;

    window.get_size(window_width, window_height);
    KLOG_WARNING("window size %d x %d, reposition now", window_width, window_height);

    //获取按钮的位置坐标
    button_allocation = get_allocation();
    get_window()->get_origin(root_x, root_y);
    orient = mate_panel_applet_get_orient(get_applet());
    switch (orient)
    {
    case MATE_PANEL_APPLET_ORIENT_UP:
        root_y -= window_height;
        break;
    case MATE_PANEL_APPLET_ORIENT_DOWN:
        root_y += button_allocation.get_height();
        break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
        root_x -= window_width;
        break;
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        root_x += button_allocation.get_width();
        break;
    }
    window.move(root_x, root_y);
}