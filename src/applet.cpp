#include <gtkmm.h>
#include <mate-panel-applet.h>
#include "kiranmenuappletbutton.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <locale.h>
#include "config.h"
#include "kiranpower.h"

#include "kiran-tasklist-widget.h"
#include "app-manager.h"
#include "core_worker.h"

#define GETTEXT_PACKAGE "kiran-menu"

bool load_resources(const std::string &resource_file) {
    try {
        auto resource = Gio::Resource::create_from_file(resource_file);
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

#define TEST
#ifndef TEST

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
           const gchar *iid,
           gpointer     data)
{
    if (strcmp(iid, "KiranMenuApplet") || strcmp(iid, "KiranTasklistApplet"))
        return FALSE;

    setlocale(LC_MESSAGES, "");
    bindtextdomain(GETTEXT_PACKAGE, DATA_DIR "/locale");
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    Gtk::Main::init_gtkmm_internals();

    if (!strcmp(iid, "KiranMenuApplet")) {
        //开始菜单插件
        if (!load_resources(MENU_RESOURCE_PATH))
            return FALSE;

        auto button = new KiranMenuAppletButton(applet);
        gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(button->gobj()));
    } else {
        MatePanelAppletFlags flags;

        //窗口切换预览插件
        if (!load_resources(TASKLIST_RESOURCE_PATH))
            return FALSE;

        flags = MATE_PANEL_APPLET_HAS_HANDLE | MATE_PANEL_APPLET_EXPAND_MAJOR | MATE_PANEL_APPLET_EXPAND_MINOR;
        mate_panel_applet_set_flags(applet, flags);
    }

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

    Kiran::init_backend_system();
    load_resources("/tmp/menu.gresource");
    Gtk::Window window;
    KiranTasklistWidget box(nullptr);


    auto context = window.get_style_context();
    context->add_class("test");
    window.set_type_hint(Gdk::WINDOW_TYPE_HINT_DOCK);
    window.set_default_size(900, -1);
    //window.set_position(Gtk::WIN_POS_CENTER);

    window.add(box);
    box.set_halign(Gtk::ALIGN_START);
    box.show_all();

    return application->run(window, argc, argv);
}
#endif
