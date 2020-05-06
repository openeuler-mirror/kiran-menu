#include "kiran-menu-window.h"
#include "kiran-search-entry.h"
#include "kiran-app-button.h"
#include "kiran-power-button.h"
#include "config.h"

struct _KiranMenuWindow {
    GObject obj;

    GtkWidget *window, *parent;
    GtkBuilder *builder;
    GResource *resource;
    GDBusProxy *proxy;
};

G_DEFINE_TYPE(KiranMenuWindow, kiran_menu_window, G_TYPE_OBJECT)

static gboolean kiran_menu_window_load_styles(KiranMenuWindow *self)
{
    GtkCssProvider *provider = gtk_css_provider_get_default();

    gtk_css_provider_load_from_resource(provider, "/kiran-menu/menu.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    return TRUE;
}

 void request_for_lock_screen() {
    GPid pid;
    gboolean ret;
    GError *error = NULL;
    const char *argv[3] = {"mate-screensaver-command", "--lock", NULL};

    ret = g_spawn_async(NULL, argv, NULL,
            G_SPAWN_CLOEXEC_PIPES | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_SEARCH_PATH,
            NULL, NULL, &pid, &error);

    if (!ret) {
        g_error("Failed to lock screen: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_menu_window_init(KiranMenuWindow *self)
{
    GError *error = NULL;
    GtkWidget *search_box, *search_entry;
    GtkWidget *top_box, *bottom_box;

    self->resource = g_resource_load(RESOURCE_PATH, &error);
    g_resources_register(self->resource);

    kiran_menu_window_load_styles(self);
    self->builder = gtk_builder_new_from_resource("/kiran-menu/ui/menu");
    self->window = GTK_WIDGET(gtk_builder_get_object(self->builder, "menu-window"));
    gtk_window_set_decorated(GTK_WINDOW(self->window), FALSE);

    search_entry = kiran_search_entry_new();
    search_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-box"));
    gtk_container_add(GTK_CONTAINER(search_box), GTK_WIDGET(search_entry));

    top_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "top-box"));
    bottom_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "bottom-box"));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(top_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/home-dir", "Home Directory", "caja")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/monitor", "System monitor", "mate-system-monitor")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/help", "Open help", "yelp")));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(bottom_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/avatar", "About me", "mate-about-me")));
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/settings", "Control center", "mate-control-center")));

    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_power_button_new()));
    gtk_widget_set_name(self->window, "menu-window");
}

void kiran_menu_window_finalize(GObject *obj)
{
    KiranMenuWindow *self = KIRAN_MENU_WINDOW(obj);

    g_object_unref(self->window);
    g_object_unref(self->builder);
    g_resources_unregister(self->resource);
}

void kiran_menu_window_class_init(KiranMenuWindowClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_menu_window_finalize;
}

KiranMenuWindow *kiran_menu_window_new(GtkWidget *parent)
{
    KiranMenuWindow *window;
    
    window = g_object_new(KIRAN_TYPE_MENU_WINDOW, NULL);
    window->parent = parent;
    return window;
}

GtkWidget *kiran_menu_window_get_window(KiranMenuWindow *window)
{
    return window->window;
}