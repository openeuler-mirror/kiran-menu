#include "kiran-menu-window.h"
#include "kiran-search-entry.h"
#include "kiran-app-button.h"
#include "kiran-power-button.h"
#include "kiran-category-item.h"
#include "kiran-app-item.h"
#include <kiran-menu-based.h>
#include "config.h"

#define FREQUENT_APPS_SHOW_MAX  4           //开始菜单中显示的最常使用应用数量

struct _KiranMenuWindow {
    GObject obj;

    GtkWidget *window, *parent;
    GtkWidget *all_apps_box, *default_apps_box;
    GtkWidget *apps_view_stack;
    GtkWidget *back_button, *all_apps_button;
    GtkBuilder *builder;
    GResource *resource;
    GDBusProxy *proxy;

    KiranMenuBased *backend;
    GHashTable *apps;
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

static void show_default_apps_page(KiranMenuWindow *self)
{
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "default-apps-page");
}

static void show_all_apps_page(KiranMenuWindow *self)
{
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "all-apps-page");
}

/**
 *
 * 加载应用程序和分类列表
 *
 */
void kiran_menu_window_load_applications(KiranMenuWindow *self)
{
    GHashTableIter iter;
    gpointer key, value;

    self->apps = kiran_menu_based_get_all_category_apps(self->backend);

    if (!self->apps) {
        g_error("The applications list is empty!!!\n");
        return;
    }

    g_hash_table_iter_init(&iter, self->apps);
    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        GList *apps, *ptr;
        gchar *category;
        KiranCategoryItem *category_item;
        KiranAppItem *app_item;

        category = key;
        apps = value;

        category_item = kiran_category_item_new(category, TRUE);
        gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(category_item));
        for (ptr = apps; ptr != NULL; ptr = ptr->next) {
            KiranApp *app = ptr->data;

            app_item = kiran_app_item_new(app);
            gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(app_item));
        }
    }
}

/**
 *
 * 加载收藏夹列表
 */
void kiran_menu_window_load_favorites(KiranMenuWindow *self)
{
    GList *fav_list, *ptr;
    KiranCategoryItem *category_item;

    category_item = kiran_category_item_new("Favorites", FALSE);
    fav_list = kiran_menu_based_get_favorite_apps(self->backend);
    gtk_container_add(GTK_CONTAINER(self->default_apps_box), GTK_WIDGET(category_item));
    g_message("%d favorite apps found\n", g_list_length(fav_list));
    for (ptr = fav_list; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_app_item_new(app);
        g_message("Found favoriate app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->default_apps_box), GTK_WIDGET(app_item));
    }
    //g_list_free_full(fav_list, g_object_unref);
}

/**
 *
 * 加载常用应用列表
 */
void kiran_menu_window_load_frequent_apps(KiranMenuWindow *self)
{
    GList *recently_apps, *ptr;
    KiranCategoryItem *category_item;


    category_item = kiran_category_item_new("Recently Used", FALSE);
    gtk_container_add(GTK_CONTAINER(self->default_apps_box), GTK_WIDGET(category_item));

    recently_apps = kiran_menu_based_get_nfrequent_apps(self->backend, FREQUENT_APPS_SHOW_MAX);
    g_message("%d recently apps found\n", g_list_length(recently_apps));

    if (!g_list_length(recently_apps)) {
        //最近使用列表为空

        GtkWidget *label = gtk_label_new("No apps available");

        gtk_widget_set_name(label, "app-empty-prompt");
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_container_add(GTK_CONTAINER(self->default_apps_box), label);
        return;
    }
    for (ptr = recently_apps; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_app_item_new(app);
        g_message("Found recently app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->default_apps_box), GTK_WIDGET(app_item));
    }

    //g_list_free_full(recently_apps, g_object_unref);
}

void kiran_menu_window_init(KiranMenuWindow *self)
{
    GError *error = NULL;
    GtkWidget *search_box, *search_entry;
    GtkWidget *top_box, *bottom_box;

    self->backend = kiran_menu_based_skeleton_new();

    self->resource = g_resource_load(RESOURCE_PATH, &error);
    if (!self->resource) {
        /**
         * 如果资源加载失败，所有界面元素都将无法获取，所以要直接退出程序
         */
        g_error("Failed to load resource '%s': %s\n", RESOURCE_PATH, error->message);
        exit(1);
    }
    g_resources_register(self->resource);

    kiran_menu_window_load_styles(self);
    self->builder = gtk_builder_new_from_resource("/kiran-menu/ui/menu");
    self->window = GTK_WIDGET(gtk_builder_get_object(self->builder, "menu-window"));
    gtk_window_set_decorated(GTK_WINDOW(self->window), FALSE);

    self->all_apps_box= GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-box"));
    self->default_apps_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "default-apps-box"));
    self->apps_view_stack = GTK_WIDGET(gtk_builder_get_object(self->builder, "apps-view-stack"));

    search_entry = kiran_search_entry_new();
    search_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-box"));
    gtk_container_add(GTK_CONTAINER(search_box), GTK_WIDGET(search_entry));

    top_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "top-box"));
    bottom_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "bottom-box"));

    self->back_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "back-button"));
    self->all_apps_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-button"));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(top_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/home-dir", "Home Directory", "caja")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/monitor", "System monitor", "mate-system-monitor")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/help", "Open help", "yelp")));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(bottom_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/avatar", "About me", "mate-about-me")));
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/settings", "Control center", "mate-control-center")));

    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_power_button_new()));
    gtk_widget_set_name(self->window, "menu-window");

    g_signal_connect_swapped(self->back_button, "clicked", G_CALLBACK(show_default_apps_page), self);
    g_signal_connect_swapped(self->all_apps_button, "clicked", G_CALLBACK(show_all_apps_page), self);


    /* 加载应用程序数据 */
    kiran_menu_window_load_frequent_apps(self);
    kiran_menu_window_load_favorites(self);
    kiran_menu_window_load_applications(self);

}

void kiran_menu_window_finalize(GObject *obj)
{
    KiranMenuWindow *self = KIRAN_MENU_WINDOW(obj);

    g_object_unref(self->window);
    g_object_unref(self->builder);
    g_resources_unregister(self->resource);
    g_hash_table_unref(self->apps);
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