#include <mate-panel-applet.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <syslog.h>
#include "kiran-menu-applet-button.h"
#include "config.h"

#define GETTEXT_PACKAGE PACKAGE_NAME

void log_handler (const gchar *log_domain,
             GLogLevelFlags log_level,
             const gchar *message,
             gpointer user_data)
{
	const char *format;
	int priority;

	switch(log_level & G_LOG_LEVEL_MASK) {
	case G_LOG_LEVEL_ERROR:
		format = "[ERROR]: %s\n";
		priority = LOG_ERR;
		break;
	case G_LOG_LEVEL_CRITICAL:
		format = "[CRITICAL]: %s\n";
		priority = LOG_CRIT;
		break;
	case G_LOG_LEVEL_WARNING:
		format = "[WARNING]: %s\n";
		priority = LOG_WARNING;
		break;
	case G_LOG_LEVEL_MESSAGE:
		format = "[MESSAGE]: %s\n";
		priority = LOG_NOTICE;
		break;
	case G_LOG_LEVEL_INFO:
		format = "[INFO]: %s\n";
		priority = LOG_INFO;
		break;
	case G_LOG_LEVEL_DEBUG:
		format = "[DEBUG]: %s\n";
		priority = LOG_DEBUG;
		break;
	default:
		format = "%s\n";
		priority = LOG_INFO;
		break;
	}

	openlog(log_domain, LOG_NDELAY | LOG_PID, LOG_USER);
	syslog(priority, format, message);
	closelog();
}

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

	g_log_set_default_handler(log_handler, NULL);

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

