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
}

void MenuAppletButton::on_toggled()
{
    if (!window.get_visible())
        window.show();
    else
        window.hide();
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