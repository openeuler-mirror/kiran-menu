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
    mate_panel_applet_set_flags(applet, MatePanelAppletFlags(MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MINOR));
    mate_panel_applet_setup_menu_from_file(applet,
                                           PACKAGE_DATA_DIR "/workspace-switcher-menu.ui.xml",
                                           create_action_group_for_workspace_switcher());

    return TRUE;
}