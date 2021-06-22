/**
 * @file          /kiran-menu/src/menu/menu-applet-button.h
 * @brief         
 * @author        songchuangfei <songchuangfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
