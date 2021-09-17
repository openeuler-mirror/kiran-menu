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
