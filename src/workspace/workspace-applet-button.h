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

#ifndef WORKSPACEAPPLETBUTTON_H
#define WORKSPACEAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "workspace-applet-window.h"

// 面板上的工作区切换按钮
class WorkspaceAppletButton : public KiranAppletButton
{
public:
    /**
     * @brief 构造函数
     * @param applet_ 所属的面板插件
     */
    WorkspaceAppletButton(MatePanelApplet *applet_);

protected:
    virtual void on_toggled() override;

private:
    WorkspaceAppletWindow applet_window; /* 工作区切换预览窗口 */
};

#endif  // WORKSPACEAPPLETBUTTON_H
