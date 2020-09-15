#ifndef KIRANMENUAPPLETBUTTON_H
#define KIRANMENUAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "kiran-menu-window.h"

class KiranMenuAppletButton : public KiranAppletButton
{
public:
    KiranMenuAppletButton(MatePanelApplet *panel_applet);
    ~KiranMenuAppletButton() override;

protected:
    virtual void on_toggled() override;

private:
    KiranMenuWindow window;                     //菜单窗口
    sigc::connection connection1;
};

#endif // KIRANMENUAPPLETBUTTON_H
