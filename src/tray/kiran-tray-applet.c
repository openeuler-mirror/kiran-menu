#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include "kiran-tray-applet.h"
#include "kiran-tray.h"
#include "kiran-sn-watcher.h"
#include "../config.h"

#define TRAY_ICON "mate-panel-notification-area"
#define LOCALE_DIR DATA_DIR "/locale/"

typedef struct _KiranTrayData KiranTrayData;

struct _KiranTrayData
{
    MatePanelApplet *applet;
    GtkWidget *tray;
    MatePanelAppletOrient  orient;
    KiranSnWatcher *watcher;
};

static void display_about_dialog      (GtkAction       *action,
                                       KiranTrayData  *kcd);

static const GtkActionEntry kiran_tray_menu_actions [] = {
	{ "KiranTrayAbout", "kiran-help-about", N_("_About"),
          NULL, NULL,
          G_CALLBACK (display_about_dialog) }
};

static void 
display_about_dialog(GtkAction       *action, 
		     KiranTrayData  *kcd)
{
        static const gchar* authors[] = {
                "wangxiaoqing <wangxiaoqing@kylinos.com.cn>",
                "songchuanfei <songchuanfei@kylinos.com.cn>",
                NULL
        };

        static const char* documenters[] = {
                "HuNan Kylin <www@kylinsec.com.cn>",
                NULL
        };

        gtk_show_about_dialog(NULL,
                "program-name", _("Tray"),
                "title", _("About Tray"),
                "authors", authors,
                "comments", _("The Tray displays the system notify icon"),
                "copyright", _("Copyright ©2020 KylinSec. All rights reserved."),
                "documenters", documenters,
                "logo-icon-name", TRAY_ICON,
                "translator-credits", _("translator-credits"),
                "version", PACKAGE_VERSION,
                "website", "http://www.kylinsec.com.cn",
                NULL);
}

static gboolean
button_press (GtkWidget      *widget,
              GdkEventButton *event,
              gpointer       user_data)
{
    if (event->button != 1)
    {
	g_signal_stop_emission_by_name (widget, "button_press_event");
    }

    return FALSE;
}

static gboolean
mouse_enter (GtkWidget      *widget,
             GdkEvent       *event,
             gpointer       user_data)
{
    mate_panel_applet_request_focus (MATE_PANEL_APPLET (widget),
		   		     gtk_get_current_event_time ()); 

    return FALSE;
}

static void
destroy_tray (GtkWidget *widget, 
	       gpointer   user_data)
{
    KiranTrayData *kcd  = user_data;

    g_clear_object (&kcd->watcher);

    g_free (kcd);
}

static void
applet_change_orient (MatePanelApplet       *applet,
                      MatePanelAppletOrient  orient,
                      KiranTrayData         *kcd)
{
    if (orient == kcd->orient)
            return;

    kcd->orient = orient;
}

gboolean
fill_tray_applet (MatePanelApplet *applet)
{
    KiranTrayData *kcd;
    GtkActionGroup *action_group;

    kcd = g_new0 (KiranTrayData, 1);

    kcd->applet = applet;
    kcd->tray = kiran_tray_new (); 

    gtk_container_add (GTK_CONTAINER (applet), kcd->tray);
    gtk_widget_show (kcd->tray);
    gtk_widget_show (GTK_WIDGET (kcd->applet));
    gtk_widget_set_size_request (kcd->tray, 80, mate_panel_applet_get_size (kcd->applet));

    kcd->watcher = kiran_sn_watcher_new ();
    g_signal_connect (G_OBJECT (kcd->tray), "button-press-event", G_CALLBACK(button_press), kcd);
    g_signal_connect (G_OBJECT (kcd->tray), "destroy", G_CALLBACK (destroy_tray), kcd);
    g_signal_connect (G_OBJECT (applet), "enter-notify-event", G_CALLBACK (mouse_enter), kcd);

    g_signal_connect (G_OBJECT (applet),
                          "change_orient",
                          G_CALLBACK (applet_change_orient),
                          kcd);

    applet_change_orient (MATE_PANEL_APPLET (applet),
                              mate_panel_applet_get_orient (MATE_PANEL_APPLET (applet)),
                              kcd);

    action_group = gtk_action_group_new ("KiranTrayApplet Menu Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group,
                                  kiran_tray_menu_actions,
                                  G_N_ELEMENTS (kiran_tray_menu_actions),
                                  kcd);
    mate_panel_applet_setup_menu (MATE_PANEL_APPLET (kcd->applet),
                                  "<menuitem name=\"Kiran Tray About Item\" action=\"KiranTrayAbout\" />",
                                  action_group);


    return TRUE;
}

#if 0
static gboolean
kiran_tray_factory (MatePanelApplet *applet,
		     const char *iid,
		     gpointer data)
{
    gboolean retval = FALSE;
    GtkCssProvider *provider;
    GdkScreen *screen;
    GdkDisplay *display;
    GFile *css_fp;


    mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_HAS_HANDLE|MATE_PANEL_APPLET_EXPAND_MINOR);

    bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
    textdomain (GETTEXT_PACKAGE);

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen,
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_fp  = g_file_new_for_path ("/usr/share/kiran-tray/kiran-tray.css");
    gtk_css_provider_load_from_file (provider, css_fp, NULL);

    if (!strcmp (iid, "KiranTrayApplet"))
	retval = fill_tray_applet (applet);

    g_object_unref (provider);
    g_object_unref (css_fp);

    return retval;
}
#endif