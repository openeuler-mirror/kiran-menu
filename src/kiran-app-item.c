#include "kiran-app-item.h"
#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>

struct _KiranAppItem
{
    GtkEventBox parent;

    GtkWidget *grid;
    GtkWidget *icon, *label;
    GtkWidget *menu;

    gboolean menu_shown;

    KiranApp *app;
};

G_DEFINE_TYPE(KiranAppItem, kiran_app_item, GTK_TYPE_EVENT_BOX);

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
    gtk_widget_set_margin_start(item->icon, 25);
    gtk_widget_set_margin_right(item->icon, 10);

    gtk_widget_show_all(item->grid);

    gtk_widget_add_events(GTK_WIDGET(item), GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
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

static void menu_detach_callback(KiranAppItem *item)
{
    g_message("%s: set item %p menu-shown to FALSE\n", __func__, item);

    item->menu_shown = FALSE;
    gtk_widget_set_state(GTK_WIDGET(item), GTK_STATE_FLAG_NORMAL);
}

static GtkWidget *create_context_menu(GtkWidget *attach)
{
    GtkWidget *menu, *menu_item;
    KiranAppItem *item = KIRAN_APP_ITEM(attach);

    menu = gtk_menu_new();

    menu_item = gtk_menu_item_new_with_label(_("Launch"));
    g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_launch), item->app);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    menu_item = gtk_menu_item_new_with_label(_("Add to desktop"));
    g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(add_to_desktop), attach);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    menu_item = gtk_menu_item_new_with_label(_("Add to favorites"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_menu_attach_to_widget(GTK_MENU(menu), attach, (GtkMenuDetachFunc)destroy_menu);
    g_signal_connect_swapped(menu, "deactivate", G_CALLBACK(menu_detach_callback), attach);

    gtk_widget_show_all(menu);

    return menu;
}

gboolean kiran_app_item_button_release(GtkWidget *widget, GdkEventButton *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);
    GdkEvent *event = (GdkEvent *)ev;
 
    g_message("%s: got button release event\n", __func__);
    if (ev->button == 3) {
        if (!item->menu)
            item->menu = create_context_menu(widget);
        
        g_message("%s: set item %p menu-shown to TRUE\n", __func__, item);
        item->menu_shown = TRUE;
        gtk_menu_popup_at_pointer(GTK_MENU(item->menu), event);
 
        return TRUE;
    }
    return FALSE;
}

gboolean kiran_app_item_enter_notify(GtkWidget *widget, GdkEventMotion *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);

    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);
    gtk_widget_queue_draw(widget);
    return FALSE;
}

gboolean kiran_app_item_leave_notify(GtkWidget *widget, GdkEventMotion *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);
    GtkStateFlags flags = gtk_widget_get_state_flags(widget);

    if (item->menu_shown) {
        return TRUE;
    }

    gtk_widget_set_state_flags(widget, flags & ~GTK_STATE_FLAG_PRELIGHT, TRUE);
    gtk_widget_queue_draw(widget);
    return FALSE;
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
    GTK_WIDGET_CLASS(kclass)->button_release_event = kiran_app_item_button_release;
    GTK_WIDGET_CLASS(kclass)->leave_notify_event = kiran_app_item_leave_notify;
    GTK_WIDGET_CLASS(kclass)->enter_notify_event = kiran_app_item_enter_notify;

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
