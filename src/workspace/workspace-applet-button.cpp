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

#include "workspace-applet-button.h"

WorkspaceAppletButton::WorkspaceAppletButton(MatePanelApplet *applet_) : KiranAppletButton(applet_)
{
    set_icon_from_resource("/kiran-workspace/icon/button");

    applet_window.signal_unmap_event().connect(
        [this](GdkEventAny *event) -> bool {
            set_active(false);
            return false;
        });
}

void WorkspaceAppletButton::on_toggled()
{
    if (get_active())
    {
        if (!applet_window.is_visible())
            applet_window.show();
    }
    else
    {
        if (applet_window.is_visible())
            applet_window.hide();
    }
}
