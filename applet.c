#include <mate-panel-applet.h>
#include <gtk/gtk.h>
#include "kiran-menu-applet-button.h"

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
		   const gchar *iid,
		   gpointer     data)
{
	GError *error = NULL;
	KiranMenuAppletButton *button;

	if (strcmp(iid, "KiranMenuApplet"))
		return FALSE;

	button = kiran_menu_applet_button_new(applet);
	gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button));
	gtk_widget_show_all(GTK_WIDGET(applet));

	return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranMenuAppletFactory",
		PANEL_TYPE_APPLET,
		"kiran_menu_applet",
		kiran_menu_applet_fill,
		NULL);

