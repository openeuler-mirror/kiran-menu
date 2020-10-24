#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "menu-applet-button.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <locale.h>
#include "../config.h"
#include "kiran-power.h"
#include <X11/Xlib.h>
#include <gtk/gtkx.h>

#include "tasklist-applet-widget.h"
#include "app-manager.h"
#include "core_worker.h"
#include "workspace-applet-button.h"
#include "global.h"

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



static void show_applet_about_dialog(const char *program_name,
                                     const char *icon_name,
                                     const char *comments)
{
    GtkWidget *dialog;

    dialog = gtk_about_dialog_new();

    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), program_name);
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), icon_name);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), comments);

    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), PACKAGE_VERSION);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
                                   "Copyright © KylinSec Co., Ltd. 2020. All rights reserved.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void open_menu_about(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Menu Applet"),
                             "kiran-menu",
                             _("Startup menu designed for Kiran Desktop"));
}


void open_tasklist_about(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Tasklist Applet"),
                             "kiran-window-switcher",
                             _("Task switcher designed for Kiran Desktop"));
}

void open_workspace_switcher_about(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Workspace switcher Applet"),
                             "kiran-workspace-switcher",
                             _("Workspace switcher designed for Kiran Desktop"));
}

GtkActionGroup *create_action_group_for_menu()
{
    GtkActionGroup *action_group;
    GtkAction *action;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_group = gtk_action_group_new("kiran-menu");
    action = gtk_action_new("KiranMenuAbout", _("About"), _("About this applet"), "gtk-about");
    gtk_action_group_add_action(action_group, action);
    g_signal_connect(action, "activate", G_CALLBACK(open_menu_about), NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS


    return action_group;
}

GtkActionGroup *create_action_group_for_tasklist()
{
    GtkActionGroup *action_group;
    GtkAction *action;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_group = gtk_action_group_new("kiran-tasklist");

    action = gtk_action_new("KiranTasklistAbout", _("About"), _("About this applet"), "gtk-about");
    gtk_action_group_add_action(action_group, action);
    g_signal_connect(action, "activate", G_CALLBACK(open_tasklist_about), NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS

    return action_group;
}

GtkActionGroup *create_action_group_for_workspace_switcher()
{
    GtkActionGroup *action_group;
    GtkAction *action;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_group = gtk_action_group_new("kiran-workspace-switcher");

    action = gtk_action_new("KiranWorkspaceSwitcherAbout", _("About"), _("About this applet"), "gtk-about");
    gtk_action_group_add_action(action_group, action);
    g_signal_connect(action, "activate", G_CALLBACK(open_workspace_switcher_about), NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS

    return action_group;
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
        Display *xdisplay = gdk_x11_get_default_xdisplay();

        auto button = Gtk::make_managed<MenuAppletButton>(applet);
        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));

        atom_mate_panel_action_kiran_menu = XInternAtom(xdisplay,
                                                        "_MATE_PANEL_ACTION_KIRAN_MENU",
                                                        FALSE);

        atom_mate_panel_action = XInternAtom(xdisplay,
                                             "_MATE_PANEL_ACTION",
                                             FALSE);

        gdk_window_add_filter(nullptr, key_event_filter, button);
        mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR);
        mate_panel_applet_setup_menu_from_file(applet,
                                               PACKAGE_DATA_DIR "/menu-menu.ui.xml",
                                               create_action_group_for_menu());
    } else if (!strcmp(iid, "KiranTasklistApplet"))  {
        int flags;
        TasklistAppletWidget *button = Gtk::make_managed<TasklistAppletWidget>(applet);

        flags = MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_EXPAND_MAJOR;
        mate_panel_applet_set_flags(applet, (MatePanelAppletFlags)flags);
        mate_panel_applet_setup_menu_from_file(applet,
                                               PACKAGE_DATA_DIR "/tasklist-menu.ui.xml",
                                               create_action_group_for_tasklist());

        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    } else if (!strcmp(iid, "KiranWorkspaceApplet")) {
        WorkspaceAppletButton *button = Gtk::make_managed<WorkspaceAppletButton>(applet);

        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
        mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR);
        mate_panel_applet_setup_menu_from_file(applet,
                                               PACKAGE_DATA_DIR "/workspace-switcher-menu.ui.xml",
                                               create_action_group_for_workspace_switcher());
    }

    gtk_widget_show_all(GTK_WIDGET(applet));


    return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranAppletFactory",
        PANEL_TYPE_APPLET,
        "kiran_applet",
        kiran_menu_applet_fill,
        NULL);
