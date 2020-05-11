#include "kiran-menu-window.h"
#include "kiran-search-entry.h"
#include "kiran-app-button.h"
#include "kiran-power-button.h"
#include "kiran-category-item.h"
#include "kiran-app-item.h"
#include <kiran-menu-based.h>
#include <glib/gi18n.h>
#include "config.h"

#define FREQUENT_APPS_SHOW_MAX  4           //开始菜单中显示的最常使用应用数量
#define NEW_APPS_SHOW_MAX       4           //开始菜单中显示的新安装应用数量

struct _KiranMenuWindow {
    GObject obj;

    GtkWidget *window, *parent;
    GtkWidget *all_apps_box, *default_apps_box, *search_results_box;
    GtkWidget *apps_view_stack, *overview_stack;
    GtkWidget *apps_overview_page, *category_overview_box;
    GtkWidget *all_apps_viewport;
    GtkWidget *back_button, *all_apps_button;
    GtkWidget *search_entry;
    GtkBuilder *builder;
    GResource *resource;
    GDBusProxy *proxy;

    const char *last_app_view;

    KiranMenuBased *backend;
    GHashTable *apps;
    GList *category_list;
    GList *favorite_apps;

    GHashTable *category_items;
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
    //更改动画切换方向，看起来更自然
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "default-apps-page");
}

static void show_all_apps_page(KiranMenuWindow *self)
{
    //更改动画切换方向，看起来更自然
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "all-apps-page");
}


static void show_apps_overview(KiranMenuWindow *self)
{
    gtk_stack_set_transition_type(GTK_STACK(self->overview_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->overview_stack), "apps-overview-page");
}


/**
 * 跳转到指定的应用分类
 *
 */
void kiran_menu_window_jump_to_category(KiranMenuWindow *self, const char *category_name)
{
    GtkAllocation item_allocation;
    KiranCategoryItem *item;
    GtkAdjustment *adjustment;

    //切换到所有应用列表
    show_apps_overview(self);
    show_all_apps_page(self);

    item = g_hash_table_lookup(self->category_items, category_name);
    if (!item) {
        g_warning("%s: Item for category '%s' not found\n", __func__, category_name);
        return;
    }

    adjustment = gtk_viewport_get_vadjustment(GTK_VIEWPORT(self->all_apps_viewport));
    gtk_widget_get_allocation(GTK_WIDGET(item), &item_allocation);
    gtk_adjustment_set_value(adjustment, item_allocation.y);
}


/**
 * 在分类选择视图中点击分类时的回调函数
 */
static void category_selected_callback(KiranMenuWindow *self, GtkButton *button)
{
    char *category_name;

    category_name = g_object_get_data(G_OBJECT(button), "category-name");

    g_message("%s: jump to category '%s'\n", __func__, category_name);
    kiran_menu_window_jump_to_category(self, category_name);
}


/**
 * 显示分类选择视图
 *
 */
static void show_category_overview(KiranMenuWindow *self, GtkButton *button)
{
    GList *ptr;
    const char *current_category;

    current_category = kiran_category_item_get_category_name(KIRAN_CATEGORY_ITEM(button));

    //清空分类选择列表
    gtk_container_foreach(GTK_CONTAINER(self->category_overview_box), (GtkCallback)gtk_widget_destroy, NULL);

    for (ptr = self->category_list; ptr != NULL; ptr = ptr->next) {
        GtkStyleContext *context;
        GtkWidget *button;

        button = gtk_button_new_with_label((char*)ptr->data);
        gtk_button_set_alignment(GTK_BUTTON(button), 0.0, 0.5);
        gtk_widget_set_hexpand(button, TRUE);
        g_object_set_data_full(G_OBJECT(button), "category-name", g_strdup(ptr->data), (GDestroyNotify)g_free);

        context = gtk_widget_get_style_context(button);
        gtk_style_context_add_class(context, "kiran-category-selector");

        gtk_container_add(GTK_CONTAINER(self->category_overview_box), button);
        gtk_widget_show(button);

        if (!strcmp((char*)ptr->data, current_category)) {
            //将当前分类设置为默认激活状态
            gtk_widget_grab_focus(button);
            g_message("%s: found same category\n", __func__);
        }
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(category_selected_callback), self);
    }
    gtk_stack_set_transition_type(GTK_STACK(self->overview_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->overview_stack), "category-overview-page");
}


/**
 * 应用搜索框停止搜索时回调函数
 *
 */
static void search_stop_callback(KiranMenuWindow *self)
{
    //返回到搜索之前的页面
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), self->last_app_view?self->last_app_view:"default-apps-page");
}


/**
 * 应用搜索框内容变化时回调函数
 *
 */
static void search_change_callback(KiranMenuWindow *self)
{
    GList *result_apps, *ptr;
    const gchar *keyword, *visible_view;
    KiranCategoryItem *category_item;

    if (gtk_entry_get_text_length(GTK_ENTRY(self->search_entry)) == 0) {
        //搜索内容为空，停止搜索，并返回上一个页面
        search_stop_callback(self);
        return;
    }

    //记录搜索前的页面，在搜索返回时使用
    visible_view = gtk_stack_get_visible_child_name(GTK_STACK(self->apps_view_stack));
    if (strcmp(visible_view, "search-results-page"))
        self->last_app_view = visible_view;

    //切换到搜索页面
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "search-results-page");

    //清空之前的搜索结果
    gtk_container_foreach(GTK_CONTAINER(self->search_results_box), (GtkCallback)gtk_widget_destroy, NULL);

    keyword = gtk_entry_get_text(GTK_ENTRY(self->search_entry));
    result_apps = kiran_menu_based_search_app(self->backend, keyword);

    if (!g_list_length(result_apps)) {
        GtkStyleContext *context;
        GtkWidget *label = gtk_label_new(_("No Apps match the results!"));

        context = gtk_widget_get_style_context(label);
        gtk_style_context_add_class(context, "search-empty-prompt");

        gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);

        gtk_container_add(GTK_CONTAINER(self->search_results_box), label);
        gtk_widget_show(label);
        return;
    }

    category_item = kiran_category_item_new(_("Search Results"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->search_results_box), GTK_WIDGET(category_item));
    for (ptr = result_apps; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_app_item_new(app);
        g_message("Found result app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->search_results_box), GTK_WIDGET(app_item));
    }
    gtk_widget_show_all(GTK_WIDGET(self->search_results_box));
    g_list_free_full(result_apps, g_object_unref);
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

        if (!g_list_length(apps))
            continue;

        //对于没有应用的分类，不做存储
        self->category_list = g_list_append(self->category_list, category);

        //添加应用分类标签
        category_item = kiran_category_item_new(category, TRUE);
        g_hash_table_insert(self->category_items, g_strdup(category), category_item);
        gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(category_item));

        //添加应用程序标签
        for (ptr = apps; ptr != NULL; ptr = ptr->next) {
            KiranApp *app = ptr->data;

            app_item = kiran_app_item_new(app);
            gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(app_item));
        }
        g_signal_connect_swapped(category_item, "clicked", G_CALLBACK(show_category_overview), self);
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

    category_item = kiran_category_item_new(_("Favorites"), FALSE);
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

    self->favorite_apps = fav_list;
}

/**
 *
 * 加载常用应用列表
 */
void kiran_menu_window_load_frequent_apps(KiranMenuWindow *self)
{
    GList *recently_apps, *ptr;
    KiranCategoryItem *category_item;


    category_item = kiran_category_item_new(_("Frequently Used"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->default_apps_box), GTK_WIDGET(category_item));

    recently_apps = kiran_menu_based_get_nfrequent_apps(self->backend, FREQUENT_APPS_SHOW_MAX);
    g_message("%d recently apps found\n", g_list_length(recently_apps));

    if (!g_list_length(recently_apps)) {
        //最近使用列表为空

        GtkWidget *label = gtk_label_new(_("No apps available"));

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

    g_list_free_full(recently_apps, g_object_unref);
}

/**
 *
 * 加载新安装应用列表
 */
void kiran_menu_window_load_new_apps(KiranMenuWindow *self)
{
    GList *new_apps, *ptr;

  KiranCategoryItem *category_item;


    category_item = kiran_category_item_new(_("New Installed"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(category_item));

    new_apps = kiran_menu_based_get_nnew_apps(self->backend, NEW_APPS_SHOW_MAX);

    g_message("%d recently apps found\n", g_list_length(new_apps));

    if (!g_list_length(new_apps)) {
        //最近使用列表为空

        GtkWidget *label = gtk_label_new(_("No apps available"));

        gtk_widget_set_name(label, "app-empty-prompt");
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_container_add(GTK_CONTAINER(self->all_apps_box), label);
        return;
    }
    for (ptr = new_apps; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_app_item_new(app);
        g_message("Found new app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(app_item));
    }

    g_list_free_full(new_apps, g_object_unref);
}

void kiran_menu_window_init(KiranMenuWindow *self)
{
    GError *error = NULL;
    GtkWidget *search_box;
    GtkWidget *top_box, *bottom_box;

    self->backend = kiran_menu_based_skeleton_get();
    self->last_app_view = NULL;
    self->category_list = NULL;
    self->category_items = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)g_object_unref);

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
    self->search_results_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-results-box"));

    self->category_overview_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "category-overview-box"));
    self->all_apps_viewport = GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-viewport"));

    self->apps_view_stack = GTK_WIDGET(gtk_builder_get_object(self->builder, "apps-view-stack"));
    self->overview_stack = GTK_WIDGET(gtk_builder_get_object(self->builder, "overview-stack"));

    self->search_entry = GTK_WIDGET(kiran_search_entry_new());
    search_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-box"));
    gtk_container_add(GTK_CONTAINER(search_box), self->search_entry);

    g_signal_connect_swapped(self->search_entry, "search-changed", G_CALLBACK(search_change_callback), self);
    g_signal_connect_swapped(self->search_entry, "stop-search", G_CALLBACK(search_stop_callback), self);

    top_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "top-box"));
    bottom_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "bottom-box"));

    self->back_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "back-button"));
    self->all_apps_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-button"));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(top_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/home-dir", _("Home Directory"), "caja")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/monitor", _("System monitor"), "mate-system-monitor")));
    gtk_container_add(GTK_CONTAINER(top_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/help", _("Open help"), "yelp")));

    gtk_orientable_set_orientation(GTK_ORIENTABLE(bottom_box), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/avatar", _("About me"), "mate-about-me")));
    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_app_button_new("/kiran-menu/sidebar/settings", _("Control center"), "mate-control-center")));

    gtk_container_add(GTK_CONTAINER(bottom_box), GTK_WIDGET(kiran_power_button_new()));
    gtk_widget_set_name(self->window, "menu-window");

    g_signal_connect_swapped(self->back_button, "clicked", G_CALLBACK(show_default_apps_page), self);
    g_signal_connect_swapped(self->all_apps_button, "clicked", G_CALLBACK(show_all_apps_page), self);


    /* 加载应用程序数据 */
    kiran_menu_window_load_frequent_apps(self);
    kiran_menu_window_load_favorites(self);
    kiran_menu_window_load_new_apps(self);
    kiran_menu_window_load_applications(self);

}

void kiran_menu_window_finalize(GObject *obj)
{
    KiranMenuWindow *self = KIRAN_MENU_WINDOW(obj);

    g_object_unref(self->window);
    g_object_unref(self->builder);
    g_resources_unregister(self->resource);
    g_hash_table_unref(self->apps);
    g_list_free_full(self->category_list, g_free);
    g_hash_table_destroy(self->category_items);
    g_list_free_full(self->favorite_apps, g_object_unref);
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
