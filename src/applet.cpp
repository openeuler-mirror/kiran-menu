#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiran-menu-applet-button.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <locale.h>
#include "config.h"
#include "kiranpower.h"

#define GETTEXT_PACKAGE "kiran-menu"

bool load_resources() {
    try {
        auto resource = Gio::Resource::create_from_file(RESOURCE_PATH);
        resource->register_global();
    } catch (const Glib::Error &e) {
        std::cerr<<"Failed to load resource file: '"<<e.what()<<"'"<<std::endl;
        return false;
    }

    try {
        auto provider = Gtk::CssProvider::create();
        provider->load_from_resource("/kiran-menu/menu.css");
        Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(),
                                                   provider,
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    } catch (const Glib::Error &e) {
        std::cerr<<"Failed to load widget styles: '"<<e.what()<<"'"<<std::endl;
        return false;
    }

    return true;
}

#ifndef TEST

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
           const gchar *iid,
           gpointer     data)
{
    if (strcmp(iid, "KiranMenuApplet"))
        return FALSE;

    setlocale(LC_MESSAGES, "");
    bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    Gtk::Main::init_gtkmm_internals();

    if (!load_resources())
	return FALSE;

    auto button = new KiranMenuAppletButton(applet);
    gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    gtk_widget_show_all(GTK_WIDGET(applet));

    return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranMenuAppletFactory",
        PANEL_TYPE_APPLET,
        "kiran_menu_applet",
        kiran_menu_applet_fill,
        NULL);
#else
int main(int argc, char *argv[]) {
    auto application = Gtk::Application::create("org.test.test");

    load_resources();
    KiranMenuWindow window;

    return application->run(window, argc, argv);
}
#endif
