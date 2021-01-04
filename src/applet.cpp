#include <iostream>
#include <locale.h>
#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <zlog_ex.h>

#include "app-manager.h"
#include "core_worker.h"
#include "log.h"

#include "menu/menu-applet.h"
#include "tasklist/tasklist-applet.h"
#include "workspace/workspace-applet.h"
#include "tray/kiran-tray-applet.h"
#include "common/kiran-power.h"
#include "common/global.h"

#include "config.h"

#define APPLET_RESOURCE_PATH   PACKAGE_DATA_DIR "/kiran-applet.gresource"

static void log_handler(const gchar *log_domain,
                        GLogLevelFlags log_level,
                        const gchar *message,
                        gpointer user_data)
{
    switch (log_level & G_LOG_LEVEL_MASK)
    {
    case G_LOG_LEVEL_DEBUG:
        dzlog_debug("[%s]: %s", log_domain, message);
        break;
    case G_LOG_LEVEL_ERROR:
        dzlog_error("[%s]: %s", log_domain, message);
        break;
    case G_LOG_LEVEL_INFO:
        dzlog_info("[%s]: %s", log_domain, message);
        break;
    case G_LOG_LEVEL_MESSAGE:
        dzlog_notice("[%s]: %s", log_domain, message);
        break;
    case G_LOG_LEVEL_WARNING:
        dzlog_warn("[%s]: %s", log_domain, message);
        break;
    case G_LOG_LEVEL_CRITICAL:
        dzlog_fatal("[%s]: %s", log_domain, message);
        break;
    default:
        break;
    }
}

static void log_init()
{
    static bool inited = false;
    const char *zlog_config = nullptr;

    if (inited)
        return;

    zlog_config = g_getenv("ZLOG_CONF");
    dzlog_init_ex(zlog_config, "kiran-session-app", "kiran-applet", "kiran-applet");

    g_log_set_default_handler(log_handler, NULL);
    inited = true;
}

static bool load_resources(const std::string &resource_file) {
    try {
        auto resource = Gio::Resource::create_from_file(resource_file);
        resource->register_global();
    } catch (const Glib::Error &e) {
        LOG_ERROR("Failed to load resource file: '%s'", e.what().c_str());
        return false;
    }

    return true;
}

static bool load_css_styles(const char *name)
{
    try {
        auto provider = Gtk::CssProvider::create();
        provider->load_from_resource(name);
        Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(),
                                                   provider,
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error &e) {
        LOG_ERROR("Failed to load style file: '%s'", e.what().c_str());
        return false;
    }
    return true;
}

static gboolean
kiran_applet_factory (MatePanelApplet *applet,
           const gchar *iid,
           gpointer     data)
{
    static int backend_inited = false;

    if (strcmp(iid, "KiranMenuApplet") &&
            strcmp(iid, "KiranTasklistApplet") &&
            strcmp(iid, "KiranWorkspaceApplet") &&
            strcmp(iid, "KiranTrayApplet")) {
        return FALSE;
    }

    setlocale(LC_MESSAGES, "");
    bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    Gtk::Main::init_gtkmm_internals();
    if (!backend_inited) {
        log_init();
        Kiran::init_backend_system();
        backend_inited = true;
    }

    if (!load_resources(APPLET_RESOURCE_PATH))
        return FALSE;

    load_css_styles("/kiran-applet/css/applet.css");

    if (!strcmp(iid, "KiranMenuApplet")) {
        //开始菜单插件
        menu_applet_fill(applet);
    } else if (!strcmp(iid, "KiranTasklistApplet"))  {
        tasklist_applet_fill(applet);
    } else if (!strcmp(iid, "KiranWorkspaceApplet")) {
        workspace_applet_fill(applet);
    } else if (!strcmp(iid, "KiranTrayApplet")) {
        fill_tray_applet(applet);
    } else {
        /* Should not reach here */
        g_warn_if_reached();
    }

    gtk_widget_show_all(GTK_WIDGET(applet));


    return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranAppletFactory",
        PANEL_TYPE_APPLET,
        "kiran_applet",
        kiran_applet_factory,
        NULL);
