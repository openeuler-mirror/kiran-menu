#include <glibmm/i18n.h>
#include "menu-applet.h"
#include "menu-applet-button.h"
#include "kiran-helper.h"
#include <log.h>
#include <gtk/gtkx.h>
#include "config.h"

static Atom atom_mate_panel_action_kiran_menu  = None;
static Atom atom_mate_panel_action  = None;

static GdkFilterReturn key_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    XEvent *x_event = (XEvent *)xevent;
    Gtk::ToggleButton *button = (Gtk::ToggleButton *)data;

    if (x_event->type == ClientMessage)
    {
        LOG_DEBUG("got client message\n");

        if (x_event->xclient.message_type == atom_mate_panel_action &&
            x_event->xclient.data.l[0] == atom_mate_panel_action_kiran_menu)
        {
            LOG_DEBUG("it is kiran menu event\n");
            button->set_active(!button->get_active());
        }
    }

    return GDK_FILTER_CONTINUE;
}

static void open_about_dialog(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Menu Applet"),
                             "kiran-menu",
                             _("Startup menu designed for Kiran Desktop"));
}

static GtkActionGroup *create_action_group()
{
    GtkActionGroup *action_group;
    GtkAction *action;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_group = gtk_action_group_new("kiran-menu");
    action = gtk_action_new("KiranMenuAbout", _("About"), _("About this applet"), "gtk-about");
    gtk_action_group_add_action(action_group, action);
    g_signal_connect(action, "activate", G_CALLBACK(open_about_dialog), NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS


    return action_group;
}

static void on_applet_finalized(gpointer data, GObject *object)
{
    gdk_window_remove_filter(nullptr, key_event_filter, data);
}

gboolean menu_applet_fill(MatePanelApplet *applet)
{
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
    mate_panel_applet_set_flags(applet, MATE_PANEL_APPLET_EXPAND_MINOR);
    mate_panel_applet_setup_menu_from_file(applet,
                                           PACKAGE_DATA_DIR "/menu-menu.ui.xml",
                                           create_action_group());

    g_object_weak_ref(G_OBJECT(applet), (GWeakNotify)on_applet_finalized, button);
    return TRUE;
}
