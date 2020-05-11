#include <mate-panel-applet.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "kiran-menu-applet-button.h"
#include "config.h"


#define GETTEXT_PACKAGE "kiran-menu"

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
		   const gchar *iid,
		   gpointer     data)
{
	GError *error = NULL;
	KiranMenuAppletButton *button;

	if (strcmp(iid, "KiranMenuApplet"))
		return FALSE;

	setlocale(LC_MESSAGES, "");
	bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

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

