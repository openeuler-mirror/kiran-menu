#ifndef WORKSPACEAPPLETBUTTON_H
#define WORKSPACEAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-applet-button.h"
#include "workspace-applet-window.h"

class WorkspaceAppletButton : public KiranAppletButton
{
public:
    WorkspaceAppletButton(MatePanelApplet *applet_);
protected:
    void on_toggled() override;

private:
    WorkspaceAppletWindow applet_window;
};

#endif // WORKSPACEAPPLETBUTTON_H
