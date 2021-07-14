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

#include "workspace-applet.h"
#include <glibmm/i18n.h>
#include "config.h"
#include "src/workspace/workspace-applet-button.h"

static void open_workspace_switcher_about(GtkAction *action, gpointer userdata)
{
    show_applet_about_dialog(_("Kiran Workspace switcher Applet"),
                             "kiran-workspace-switcher",
                             _("Workspace switcher designed for Kiran Desktop"));
}

static GtkActionGroup *create_action_group_for_workspace_switcher()
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

gboolean workspace_applet_fill(MatePanelApplet *applet)
{
    WorkspaceAppletButton *button = Gtk::make_managed<WorkspaceAppletButton>(applet);

    gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    mate_panel_applet_set_flags(applet, MATE_PANEL_APPLET_EXPAND_MINOR);
    mate_panel_applet_setup_menu_from_file(applet,
                                           PACKAGE_DATA_DIR "/workspace-switcher-menu.ui.xml",
                                           create_action_group_for_workspace_switcher());

    return TRUE;
}