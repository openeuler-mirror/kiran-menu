/**
 * @file workspace-applet-button.h
 * @brief 面板上的工作区切换按钮
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#ifndef WORKSPACEAPPLETBUTTON_H
#define WORKSPACEAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "workspace-applet-window.h"

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
    WorkspaceAppletWindow applet_window;        /* 工作区切换预览窗口 */
};

#endif // WORKSPACEAPPLETBUTTON_H
