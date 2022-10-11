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

#pragma once

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "menu-applet-window.h"

class MenuAppletButton : public KiranAppletButton
{
public:
    MenuAppletButton(MatePanelApplet *panel_applet);

    /**
     * @brief 根据当前插件窗口大小和面板按钮位置，重新定位插件窗口
     */
    void reposition_applet_window();

protected:
    virtual void on_toggled() override;

private:
    MenuAppletWindow window;  //菜单窗口
};
