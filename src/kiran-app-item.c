#include "kiran-app-item.h"
#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>

#include "kiran-app-wrapper.h"

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

enum {
    SIGNAL_APP_LAUNCHED = 0,        //应用已启动信号
    SIGNAL_MAX
};

static guint signals[SIGNAL_MAX];


gboolean kiran_app_item_update(KiranAppItem *item)
{
    GIcon *icon;
    const char *data;

    if (!item->app)
        return FALSE;

    if (!KIRAN_IS_APP(item->app)) {
        g_warning("%s: object %p is not an KiranApp object\n", __func__, item->app);
        return FALSE;
    }

    icon = kiran_app_get_icon(KIRAN_APP(item->app));
    if (icon) {
        gtk_image_set_pixel_size(GTK_IMAGE(item->icon), 24);
        gtk_image_set_from_gicon(GTK_IMAGE(item->icon), icon, GTK_ICON_SIZE_LARGE_TOOLBAR);
    } else {
        //use fallback icon
        gtk_image_set_from_icon_name(GTK_IMAGE(item->icon), "application-x-executable.png", GTK_ICON_SIZE_LARGE_TOOLBAR);
    }

    data = kiran_app_get_locale_name(KIRAN_APP(item->app));
    gtk_label_set_text(GTK_LABEL(item->label), data);
    gtk_widget_set_tooltip_text(GTK_WIDGET(item), kiran_app_get_locale_comment(KIRAN_APP(item->app)));

    return TRUE;
}

void kiran_app_item_init(KiranAppItem *item)
{
    GtkStyleContext *context;
    GValue value = G_VALUE_INIT;
    GtkBorder padding;
    GtkIconTheme *icon_theme;
    int min_width, min_height, icon_spacing;

    icon_theme = gtk_icon_theme_get_default();
    context = gtk_widget_get_style_context(GTK_WIDGET(item));
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


    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    g_value_init(&value, G_TYPE_INT);
    gtk_style_context_get_style_property(context, "icon-spacing", &value);
    icon_spacing = g_value_get_int(&value);

    g_value_unset(&value);
    gtk_style_context_get_property(context, "min-width", GTK_STATE_FLAG_NORMAL, &value);
    min_width = g_value_get_int(&value);

    g_value_unset(&value);
    gtk_style_context_get_property(context, "min-height", GTK_STATE_FLAG_NORMAL, &value);
    min_height = g_value_get_int(&value);

    gtk_widget_set_margin_end(item->icon, icon_spacing);
    gtk_widget_set_margin_start(item->icon, padding.left);
    gtk_widget_set_size_request(GTK_WIDGET(item), min_width, min_height);

    gtk_widget_show_all(item->grid);

    gtk_widget_add_events(GTK_WIDGET(item), GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    gtk_widget_set_can_focus(GTK_WIDGET(item), TRUE);
    gtk_widget_set_has_window(GTK_WIDGET(item), TRUE);

    g_signal_connect_swapped(icon_theme, "changed", G_CALLBACK(kiran_app_item_update), item);
}

static void destroy_menu(GtkWidget *widget, GtkMenu *menu)
{
    g_printerr("destroy menu now\n");
    gtk_widget_destroy(GTK_WIDGET(menu));
}

static void menu_detach_callback(KiranAppItem *item)
{
    GtkStateFlags flags;

    g_message("%s: set item %p menu-shown to FALSE\n", __func__, item);

    item->menu_shown = FALSE;
    flags = gtk_widget_get_state_flags(GTK_WIDGET(item));
    gtk_widget_set_state_flags(GTK_WIDGET(item), flags & ~GTK_STATE_FLAG_PRELIGHT, TRUE);
}

/**
 * "添加到桌面"回调函数
 */
static void kiran_app_item_add_to_desktop_callback(KiranAppItem *item)
{
    g_message("%s: add '%s' onto desktop\n", __func__, kiran_app_get_desktop_id(item->app));
    if (!kiran_app_add_to_desktop(item->app))
        g_critical("Failed to add app '%s' to desktop\n", kiran_app_get_desktop_id(item->app));
}

/**
 * app"启动"回调函数
 */
static void kiran_app_item_launch_app_callback(KiranAppItem *self)
{

    g_message("%s: app '%s' launched\n", __func__, kiran_app_get_desktop_id(self->app));
    if (kiran_app_launch(self->app))
        g_signal_emit(self, signals[SIGNAL_APP_LAUNCHED], 0);
}

/**
 * 创建应用程序右键菜单
 */
GtkWidget *create_context_menu(GtkWidget *attach)
{
    GtkWidget *menu, *menu_item;
    KiranAppItem *item = KIRAN_APP_ITEM(attach);

    menu = gtk_menu_new();

    menu_item = gtk_menu_item_new_with_label(_("Launch"));
    g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_item_launch_app_callback), item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    menu_item = gtk_menu_item_new_with_label(_("Add to desktop"));
    g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_item_add_to_desktop_callback), item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    if (!kiran_app_is_favorite(item->app)) {
        menu_item = gtk_menu_item_new_with_label(_("Add to favorites"));
        g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_add_to_favorite), item->app);
    } else {
        menu_item = gtk_menu_item_new_with_label(_("Remove from favorites"));
        g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_remove_from_favorite), item->app);
    }
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
        if (item->menu)
            gtk_widget_destroy(item->menu);
        item->menu = create_context_menu(widget);
        
        g_message("%s: set item %p menu-shown to TRUE\n", __func__, item);
        item->menu_shown = TRUE;
        gtk_menu_popup_at_pointer(GTK_MENU(item->menu), event);
 
        return TRUE;
    } else {
        kiran_app_item_launch_app_callback(item);
    }
    return FALSE;
}

gboolean kiran_app_item_key_release(GtkWidget *widget, GdkEventKey *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);

    switch(ev->keyval) {
    case GDK_KEY_Menu:
        if (item->menu)
            gtk_widget_destroy(item->menu);
        item->menu = create_context_menu(widget);

        g_message("%s: set item %p menu-shown to TRUE\n", __func__, item);
        item->menu_shown = TRUE;

        do {
            GdkWindow *window;
            GtkAllocation allocation;

            window = gtk_widget_get_window(widget);
            gtk_widget_get_allocation(widget, &allocation);

            allocation.x = 0;
            allocation.y = 0;
            gtk_menu_popup_at_rect(GTK_MENU(item->menu), window, &allocation, GDK_GRAVITY_CENTER, GDK_GRAVITY_NORTH_WEST, (GdkEvent *)ev);
        } while (0);
        break;
    case GDK_KEY_Return:
        kiran_app_item_launch_app_callback(item);
        break;
    default:
        break;
    }
    return FALSE;
}

gboolean kiran_app_item_enter_notify(GtkWidget *widget, GdkEventCrossing *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);

    gtk_widget_grab_focus(widget);
    return FALSE;
}

gboolean kiran_app_item_leave_notify(GtkWidget *widget, GdkEventCrossing *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);
    GtkStateFlags flags = gtk_widget_get_state_flags(widget);

    if (item->menu_shown)
        return TRUE;

    if (gtk_widget_has_focus(widget))
        return FALSE;
    gtk_widget_set_state_flags(widget, flags & ~GTK_STATE_FLAG_FOCUSED, TRUE);
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

void kiran_app_item_class_init(KiranAppItemClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_app_item_finalize;
    GTK_WIDGET_CLASS(kclass)->button_release_event = kiran_app_item_button_release;
    GTK_WIDGET_CLASS(kclass)->key_release_event = kiran_app_item_key_release;
    GTK_WIDGET_CLASS(kclass)->leave_notify_event = kiran_app_item_leave_notify;
    GTK_WIDGET_CLASS(kclass)->enter_notify_event = kiran_app_item_enter_notify;

    GParamSpec *style_pspec = g_param_spec_int("icon-spacing",
            "icon-spacing", "Right margin of icon image", 0, 100, 0, G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

    signals[SIGNAL_APP_LAUNCHED] = g_signal_new("app-launched", G_TYPE_FROM_CLASS(kclass),
            G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

    gtk_widget_class_install_style_property(GTK_WIDGET_CLASS(kclass),
            g_param_spec_int("icon-spacing",
                             "icon-spacing",
                            "Right margin of icon image",
                            0, 100, 0,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
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
