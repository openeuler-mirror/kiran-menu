#include "showdesktop-applet.h"
#include "showdesktop-applet-button.h"
#include "kiran-helper.h"

gboolean showdesktop_applet_fill(MatePanelApplet *applet)
{
    auto button = Gtk::make_managed<ShowDesktopAppletButton>(applet);

    gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));

    mate_panel_applet_set_flags(applet, MATE_PANEL_APPLET_EXPAND_MINOR);
    return TRUE;
}