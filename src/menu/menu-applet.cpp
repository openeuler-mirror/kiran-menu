/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "menu-applet.h"
#include <glibmm/i18n.h>
#include "config.h"
#include "kiran-helper.h"
#include "lib/base.h"
#include "menu-applet-button.h"

//
#include <gtk/gtkx.h>

static Atom atom_mate_panel_action_kiran_menu = None;
static Atom atom_mate_panel_action = None;

static GdkFilterReturn key_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    XEvent *x_event = (XEvent *)xevent;
    Gtk::ToggleButton *button = (Gtk::ToggleButton *)data;

    if (x_event->type == ClientMessage)
    {
        KLOG_DEBUG("got client message\n");

        if (x_event->xclient.message_type == atom_mate_panel_action &&
            x_event->xclient.data.l[0] == (long int)atom_mate_panel_action_kiran_menu)
        {
            KLOG_DEBUG("it is kiran menu event\n");
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
