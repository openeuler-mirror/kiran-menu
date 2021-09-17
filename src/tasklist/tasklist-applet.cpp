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