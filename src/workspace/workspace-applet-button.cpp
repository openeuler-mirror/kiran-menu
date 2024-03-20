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

#include "workspace-applet-button.h"

WorkspaceAppletButton::WorkspaceAppletButton(MatePanelApplet *applet_) : KiranAppletButton(applet_)
{
    set_icon_from_theme("kiran-workspace-switcher");

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
