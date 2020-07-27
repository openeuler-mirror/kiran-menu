#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-menu-applet-button.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <locale.h>
#include "config.h"
#include "kiranpower.h"

#include "kiran-tasklist-widget.h"
#include "app-manager.h"
#include "core_worker.h"

#define GETTEXT_PACKAGE "kiran-applet"

bool load_resources(const std::string &resource_file) {
    try {
        auto resource = Gio::Resource::create_from_file(resource_file);
        resource->register_global();
    } catch (const Glib::Error &e) {
        std::cerr<<"Failed to load resource file: '"<<e.what()<<"'"<<std::endl;
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
        std::cerr<<"Failed to load widget styles: '"<<e.what()<<"'"<<std::endl;
        return false;
    }
    return true;
}

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
           const gchar *iid,
           gpointer     data)
{
    static int backend_inited = false;

    if (strcmp(iid, "KiranMenuApplet") && strcmp(iid, "KiranTasklistApplet")) {
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

        auto button = new KiranMenuAppletButton(applet);
        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    } else {
        int flags;

        //窗口切换预览插件
        g_message("loading tasklist applet\n");
        KiranTasklistWidget *button = Gtk::manage(new KiranTasklistWidget(applet));
        flags = MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_EXPAND_MAJOR;
        mate_panel_applet_set_flags(applet, (MatePanelAppletFlags)flags);

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
