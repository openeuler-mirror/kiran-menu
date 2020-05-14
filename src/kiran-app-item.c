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
    gboolean is_favorite;

    KiranApp *app;
};

G_DEFINE_TYPE(KiranAppItem, kiran_app_item, GTK_TYPE_EVENT_BOX);

enum
{
    PROPERTY_IS_FAVORITE = 1,
    PROPERTY_MAX
};

enum {
    SIGNAL_APP_LAUNCHED = 0,        //应用已启动信号
    SIGNAL_MAX
};

static GParamSpec *property_pspecs[PROPERTY_MAX] = {0};
static guint signals[SIGNAL_MAX];

void kiran_app_item_init(KiranAppItem *item)
{
    GtkStyleContext *context;
    GValue value = G_VALUE_INIT;
    GtkBorder padding;

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

    g_value_init(&value, G_TYPE_INT);
    gtk_style_context_get_style_property(context, "icon-spacing", &value);
    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    gtk_widget_set_margin_end(item->icon, g_value_get_int(&value));
    gtk_widget_set_margin_start(item->icon, padding.left);

    gtk_widget_show_all(item->grid);

    gtk_widget_add_events(GTK_WIDGET(item), GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
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

GtkWidget *create_context_menu(GtkWidget *attach);
/**
 * "添加到收藏夹"回调函数
 */
static void kiran_app_item_add_to_favorite_callback(KiranAppItem *self)
{
    if (kiran_app_add_to_favorite(self->app) && !self->is_favorite) {
        //重新生成右键菜单
        gtk_menu_detach(GTK_MENU(self->menu));
        self->is_favorite = TRUE;
        self->menu = create_context_menu(GTK_WIDGET(self));
    } else
        g_critical("Failed to add app '%s' to favorite list\n", kiran_app_get_desktop_id(self->app));
}

/**
 * "添加到桌面"回调函数
 */
static void kiran_app_item_add_to_desktop_callback(KiranAppItem *item)
{
    if (!kiran_app_add_to_favorite(item->app))
        g_critical("Failed to add app '%s' to desktop\n", kiran_app_get_desktop_id(item->app));
}


/**
 * "从收藏夹中移除"回调函数
 */
static void kiran_app_item_remove_from_favorite_callback(KiranAppItem *self)
{
    if (kiran_app_remove_from_favorite(self->app) && self->is_favorite) {
        //重新生成右键菜单
        gtk_menu_detach(GTK_MENU(self->menu));
        self->is_favorite = FALSE;
        self->menu = create_context_menu(GTK_WIDGET(self));
    } else
        g_critical("Failed to remove app '%s' from favorite list\n", kiran_app_get_desktop_id(self->app));
}

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

    if (!item->is_favorite) {
        menu_item = gtk_menu_item_new_with_label(_("Add to favorites"));
        g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_item_add_to_favorite_callback), item);
    } else {
        menu_item = gtk_menu_item_new_with_label(_("Remove from favorites"));
        g_signal_connect_swapped(menu_item, "activate", G_CALLBACK(kiran_app_item_remove_from_favorite_callback), item);
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
        if (!item->menu)
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

gboolean kiran_app_item_enter_notify(GtkWidget *widget, GdkEventCrossing *ev)
{
    KiranAppItem *item = KIRAN_APP_ITEM(widget);

    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);
    gtk_widget_queue_draw(widget);
    return FALSE;
}

gboolean kiran_app_item_leave_notify(GtkWidget *widget, GdkEventCrossing *ev)
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

    item->is_favorite = kiran_app_is_favorite(item->app);

    return TRUE;
}

void kiran_app_item_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec)
{
    KiranAppItem *item = KIRAN_APP_ITEM(object);

    switch (property_id) {
    case PROPERTY_IS_FAVORITE:
        do {
            gboolean data = g_value_get_boolean(value);

            if (data != item->is_favorite)
                item->is_favorite = data;
        } while (0);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

void kiran_app_item_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec)
{
    KiranAppItem *item = KIRAN_APP_ITEM(object);

    switch (property_id) {
    case PROPERTY_IS_FAVORITE:
        g_value_set_boolean(value, item->is_favorite);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

void kiran_app_item_class_init(KiranAppItemClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_app_item_finalize;
    G_OBJECT_CLASS(kclass)->set_property = kiran_app_item_set_property;
    G_OBJECT_CLASS(kclass)->get_property = kiran_app_item_get_property;
    GTK_WIDGET_CLASS(kclass)->button_release_event = kiran_app_item_button_release;
    GTK_WIDGET_CLASS(kclass)->leave_notify_event = kiran_app_item_leave_notify;
    GTK_WIDGET_CLASS(kclass)->enter_notify_event = kiran_app_item_enter_notify;

    property_pspecs[PROPERTY_IS_FAVORITE] = g_param_spec_boolean("is-favorite",
            "is-favorite", "In favorite apps list",
            FALSE, G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE);

    GParamSpec *style_pspec = g_param_spec_int("icon-spacing",
            "icon-spacing", "Right margin of icon image", 0, 100, 0, G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

    signals[SIGNAL_APP_LAUNCHED] = g_signal_new("app-launched", G_TYPE_FROM_CLASS(kclass),
            G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

    g_object_class_install_property(G_OBJECT_CLASS(kclass), PROPERTY_IS_FAVORITE, property_pspecs[PROPERTY_IS_FAVORITE]);
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
