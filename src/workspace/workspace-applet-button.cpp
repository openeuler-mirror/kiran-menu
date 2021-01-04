#include "workspace-applet-button.h"

WorkspaceAppletButton::WorkspaceAppletButton(MatePanelApplet *applet_):
    KiranAppletButton(applet_)
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
    if (get_active()) {
        if (!applet_window.is_visible())
            applet_window.show();
    } else {
        if (applet_window.is_visible())
            applet_window.hide();
    }
}
