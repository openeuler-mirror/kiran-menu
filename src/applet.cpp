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

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <locale.h>
#include <iostream>

#include "app-manager.h"
#include "core_worker.h"
#include "lib/base.h"

#include "common/global.h"
#include "common/kiran-power.h"
#include "menu/menu-applet.h"
#include "showdesktop/showdesktop-applet.h"
#include "tasklist/tasklist-applet.h"
#include "tray/kiran-tray-applet.h"
#include "workspace/workspace-applet.h"

#include "config.h"

#define APPLET_RESOURCE_PATH PACKAGE_DATA_DIR "/kiran-applet.gresource"

static bool load_resources(const std::string &resource_file)
{
    try
    {
        auto resource = Gio::Resource::create_from_file(resource_file);
        resource->register_global();
    }
    catch (const Glib::Error &e)
    {
        KLOG_ERROR("Failed to load resource file: '%s'", e.what().c_str());
        return false;
    }

    return true;
}

static bool load_css_styles(const char *name)
{
    try
    {
        auto provider = Gtk::CssProvider::create();
        provider->load_from_resource(name);
        Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(),
                                                   provider,
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    catch (const Glib::Error &e)
    {
        KLOG_ERROR("Failed to load style file: '%s'", e.what().c_str());
        return false;
    }
    return true;
}

static gboolean
kiran_applet_factory(MatePanelApplet *applet,
                     const gchar *iid,
                     gpointer data)
{
    static int backend_inited = false;

    setlocale(LC_MESSAGES, "");
    bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Gtk::Main::init_gtkmm_internals();
    if (!backend_inited)
    {
        klog_gtk3_init(std::string(), "kylinsec-session", "kiran-applet", "kiran-applet");
        Kiran::init_backend_system();
        backend_inited = true;
    }

    if (!load_resources(APPLET_RESOURCE_PATH))
        return FALSE;

    load_css_styles("/kiran-applet/css/applet.css");

    if (!strcmp(iid, "KiranMenuApplet"))
    {
        //开始菜单插件
        menu_applet_fill(applet);
    }
    else if (!strcmp(iid, "KiranTasklistApplet"))
    {
        tasklist_applet_fill(applet);
    }
    else if (!strcmp(iid, "KiranWorkspaceApplet"))
    {
        workspace_applet_fill(applet);
    }
    else if (!strcmp(iid, "KiranShowDesktopApplet"))
    {
        showdesktop_applet_fill(applet);
    }
    else if (!strcmp(iid, "KiranTrayApplet"))
    {
        fill_tray_applet(applet);
    }
    else
    {
        /* Should not reach here */
        g_warn_if_reached();
    }

    if (!strcmp(iid, "KiranTrayApplet"))
        gtk_widget_show(GTK_WIDGET(applet));
    else
        gtk_widget_show_all(GTK_WIDGET(applet));

    return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranAppletFactory",
                                      PANEL_TYPE_APPLET,
                                      "kiran_applet",
                                      kiran_applet_factory,
                                      NULL);
