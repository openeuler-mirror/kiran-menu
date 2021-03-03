#ifndef MENU_APPLET_BUTTON_H
#define MENU_APPLET_BUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "menu-applet-window.h"

class MenuAppletButton : public KiranAppletButton
{
public:
    MenuAppletButton(MatePanelApplet *panel_applet);

protected:
    virtual void on_toggled() override;

private:
    MenuAppletWindow window;                     //菜单窗口
};

#endif // MENU_APPLET_BUTTON_H
