/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "tasklist-applet.h"
#include <glibmm/i18n.h>
#include "tasklist-applet-widget.h"

#include "config.h"

static void open_tasklist_about(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Tasklist Applet"),
                             "kiran-window-switcher",
                             _("Task switcher designed for Kiran Desktop"));
}

static GtkActionGroup *create_action_group_for_tasklist()
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

gboolean tasklist_applet_fill(MatePanelApplet *applet)
{
    int flags;
    TasklistAppletWidget *button = Gtk::make_managed<TasklistAppletWidget>(applet);

    flags = MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_EXPAND_MAJOR;
    mate_panel_applet_set_flags(applet, (MatePanelAppletFlags)flags);
    mate_panel_applet_setup_menu_from_file(applet,
                                           PACKAGE_DATA_DIR "/tasklist-menu.ui.xml",
                                           create_action_group_for_tasklist());

    gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    return TRUE;
}