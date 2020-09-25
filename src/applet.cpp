#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-menu-applet-button.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <locale.h>
#include "../config.h"
#include "kiranpower.h"
#include <X11/Xlib.h>
#include <gtk/gtkx.h>

#include "kiran-tasklist-widget.h"
#include "app-manager.h"
#include "core_worker.h"
#include "workspace-applet-button.h"

#define APPLET_RESOURCE_PATH   PACKAGE_DATA_DIR "/kiran-applet.gresource"
#define GETTEXT_PACKAGE		   PACKAGE_NAME

static Atom atom_mate_panel_action_kiran_menu  = None;
static Atom atom_mate_panel_action  = None;

bool load_resources(const std::string &resource_file) {
    try {
        auto resource = Gio::Resource::create_from_file(resource_file);
        resource->register_global();
    } catch (const Glib::Error &e) {
        g_error("Failed to load resource file: '%s'", e.what().c_str());
        return false;
    }

    return true;
}

bool load_css_styles(const char *name)
{
    try {
        auto provider = Gtk::CssProvider::create();
        provider->load_from_resource(name);
        Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(),
                                                   provider,
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error &e) {
        g_error("Failed to load style file: '%s'", e.what().c_str());
        return false;
    }
    return true;
}

static GdkFilterReturn key_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    XEvent *x_event = (XEvent *)xevent;
    Gtk::ToggleButton *button = (Gtk::ToggleButton *)data;

    if (x_event->type == ClientMessage)
    {
        g_debug("got client message\n");

        if (x_event->xclient.message_type == atom_mate_panel_action &&
            x_event->xclient.data.l[0] == atom_mate_panel_action_kiran_menu)
        {
            g_debug("it is kiran menu event\n");
            button->set_active(!button->get_active());
        }
    }

    return GDK_FILTER_CONTINUE;
}

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
           const gchar *iid,
           gpointer     data)
{
    static int backend_inited = false;

    if (strcmp(iid, "KiranMenuApplet") &&
            strcmp(iid, "KiranTasklistApplet") &&
            strcmp(iid, "KiranWorkspaceApplet")) {
        g_warning("not match id\n");
        return FALSE;
    }

    setlocale(LC_MESSAGES, "");
    bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR);

    Gtk::Main::init_gtkmm_internals();
    if (!backend_inited) {
    	Kiran::init_backend_system();
	backend_inited = true;
    }

    if (!load_resources(APPLET_RESOURCE_PATH))
        return FALSE;

    load_css_styles("/kiran-applet/applet.css");

    if (!strcmp(iid, "KiranMenuApplet")) {
        //开始菜单插件

        auto button = new KiranMenuAppletButton(applet);
        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
        do {
            Display *xdisplay = nullptr;

            xdisplay = gdk_x11_get_default_xdisplay();
            atom_mate_panel_action_kiran_menu =
                XInternAtom(xdisplay,
                        "_MATE_PANEL_ACTION_KIRAN_MENU",
                        FALSE);

            atom_mate_panel_action =
                XInternAtom(xdisplay,
                        "_MATE_PANEL_ACTION",
                        FALSE);

            gdk_window_add_filter(nullptr, key_event_filter, button);
        } while (0);
    } else if (!strcmp(iid, "KiranTasklistApplet"))  {
        int flags;

        //窗口切换预览插件
        g_message("loading tasklist applet\n");
        KiranTasklistWidget *button = Gtk::manage(new KiranTasklistWidget(applet));
        flags = MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_EXPAND_MAJOR;
        mate_panel_applet_set_flags(applet, (MatePanelAppletFlags)flags);

        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    } else if (!strcmp(iid, "KiranWorkspaceApplet")) {
        WorkspaceAppletButton *button = Gtk::manage(new WorkspaceAppletButton(applet));
        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    }

    gtk_widget_show_all(GTK_WIDGET(applet));


    return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranAppletFactory",
        PANEL_TYPE_APPLET,
        "kiran_applet",
        kiran_menu_applet_fill,
        NULL);
