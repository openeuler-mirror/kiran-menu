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
    void on_window_hide_cb();

private:
    MenuAppletWindow window;  //菜单窗口
};
