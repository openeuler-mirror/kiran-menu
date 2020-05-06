#include "kiran-app-item.h"
#include <gio/gdesktopappinfo.h>

struct _KiranAppItem
{
    GtkButton parent;

    GtkWidget *grid;
    GtkWidget *icon, *label;
    GtkWidget *menu;

    KiranApp *app;
};

G_DEFINE_TYPE(KiranAppItem, kiran_app_item, GTK_TYPE_BUTTON);

enum
{
    PROPERTY_DESKTOP_FILE = 1,
    PROPERTY_MAX
};

static GParamSpec *param_specs[PROPERTY_MAX] = {0};

void kiran_app_item_init(KiranAppItem *item)
{
    item->grid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    item->icon = gtk_image_new();
    item->label = gtk_label_new("app name");

    gtk_orientable_set_orientation(GTK_ORIENTABLE(item->grid), GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(item->grid), item->icon);
    gtk_container_add(GTK_CONTAINER(item->grid), item->label);
    gtk_container_add(GTK_CONTAINER(item), item->grid);

    gtk_label_set_ellipsize(GTK_LABEL(item->label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_hexpand(item->label, TRUE);
    gtk_widget_set_halign(item->label, GTK_ALIGN_START);

    gtk_widget_show_all(item->grid);
}

static void destroy_menu(GtkWidget *widget, GtkMenu *menu)
{
    g_printerr("destroy menu now\n");
    gtk_widget_destroy(GTK_WIDGET(menu));
}

static void add_to_desktop(KiranAppItem *item)
{
    gchar *command = NULL;
    GError *error = NULL;
    gint status;

    g_printerr("add desktop item\n");
    command = g_strdup_printf("xdg-desktop-icon install %s --novendor", kiran_app_get_file_name(item->app));
    g_assert(command != NULL);

    if (!g_spawn_command_line_sync(command, NULL, NULL, &status, &error)) {
        g_warning("Failed to call command: %s\n", error->message);
        g_error_free(error);
    } else {
        if (status) {
            g_warning("command '%s' exited with code %d\n", command, status);
        }
    }

    g_free(command);
}

static GtkWidget *create_context_menu(GtkWidget *attach)
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new();
    menu_item = gtk_menu_item_new_with_label("Add to desktop");
    g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(add_to_desktop), attach);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Add to favorites");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_menu_attach_to_widget(GTK_MENU(menu), attach, (GtkMenuDetachFunc)destroy_menu);
    //g_signal_connect(menu, "deactivate", G_CALLBACK(gtk_menu_detach), NULL);

    gtk_widget_show_all(menu);

    return menu;
}

gboolean kiran_app_item_button_press(GtkWidget *widget, GdkEventButton *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);
    GdkEvent *event = (GdkEvent *)ev;

    if (gdk_event_triggers_context_menu(event)) {
        if (!item->menu)
            item->menu = create_context_menu(widget);
        
        gtk_menu_popup_at_pointer(GTK_MENU(item->menu), event);
    }
    return GTK_WIDGET_CLASS(kiran_app_item_parent_class)->button_press_event(widget, ev);
}

void kiran_app_item_button_clicked(GtkButton *button) {
    KiranAppItem *item = KIRAN_APP_ITEM(button);
    if (!kiran_app_launch (item->app)) {
        g_error("Failed to launch application '%s'\n", kiran_app_get_name(item->app));
    }
}

void kiran_app_item_finalize(GObject *obj)
{
    KiranAppItem *item = KIRAN_APP_ITEM(obj);

    if (item->app)
        g_object_unref(item->app);

    G_OBJECT_CLASS(kiran_app_item_parent_class)->finalize(obj);
}

static gboolean kiran_app_item_update(KiranAppItem *item)
{
    GIcon *icon;
    const char *data;

    if (!item->app)
        return FALSE;
    
    //FIXME should call kiran_app_XXX()
#if 1
    icon = kiran_app_get_icon(KIRAN_APP(item->app));
#else
    data = "firefox";
#endif

    if (icon) {
        if (G_IS_FILE_ICON(icon))
            g_message("got file icon '%s'\n", g_icon_to_string(icon));
        gtk_image_set_pixel_size(GTK_IMAGE(item->icon), 24);
        gtk_image_set_from_gicon(GTK_IMAGE(item->icon), icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
    } else {
        gtk_image_set_from_icon_name(GTK_IMAGE(item->icon), "application-x-executable.png", GTK_ICON_SIZE_LARGE_TOOLBAR);
    }

    data = kiran_app_get_locale_name(KIRAN_APP(item->app));
    gtk_label_set_text(GTK_LABEL(item->label), data);
    gtk_widget_set_tooltip_text(GTK_WIDGET(item), kiran_app_get_locale_comment(KIRAN_APP(item->app)));

    return TRUE;
}

void kiran_app_item_class_init(KiranAppItemClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_app_item_finalize;
    GTK_WIDGET_CLASS(kclass)->button_press_event = kiran_app_item_button_press;
    GTK_BUTTON_CLASS(kclass)->clicked = kiran_app_item_button_clicked;

    g_object_class_install_property(G_OBJECT_CLASS(kclass), PROPERTY_DESKTOP_FILE, param_specs[PROPERTY_DESKTOP_FILE]);
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(kclass), "kiran-app-item");
}

KiranAppItem *kiran_app_item_new(KiranApp *app)
{
    KiranAppItem *item;

    if (!app)
        return NULL;

    item = g_object_new(KIRAN_TYPE_APP_ITEM, NULL);
    item->app = g_object_ref(app);

    kiran_app_item_update(item);
    return item;
}
