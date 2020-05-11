/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 19:59:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 14:42:02
 * @Description  : 开始菜单类
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-skeleton.c
 */
#include "lib/kiran-menu-skeleton.h"

#include <gio/gio.h>

#include "lib/kiran-menu-category.h"
#include "lib/kiran-menu-favorite.h"
#include "lib/kiran-menu-search.h"
#include "lib/kiran-menu-system.h"
#include "lib/kiran-menu-usage.h"

struct _KiranMenuSkeleton
{
    GApplication parent;

    KiranMenuSystem *system;

    KiranMenuUsage *usage;

    KiranMenuSearch *search;

    KiranMenuFavorite *favorite;

    KiranMenuCategory *category;
};

enum
{
    INSTALLED_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static GList *trans_ids_to_apps(KiranMenuSkeleton *skeleton,
                                GList *desktop_ids)
{
    GList *apps = NULL;

    for (GList *l = desktop_ids; l != NULL; l = l->next)
    {
        gchar *desktop_id = l->data;
        KiranMenuApp *menu_app =
            kiran_menu_system_lookup_app(skeleton->system, desktop_id);
        if (menu_app)
        {
            apps = g_list_append(apps, g_object_ref(KIRAN_APP(menu_app)));
        }
    }
    return apps;
}

static GList *kiran_menu_skeleton_search_app(KiranMenuBased *self,
                                             const char *keyword,
                                             gboolean ignore_case)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    GList *apps = kiran_menu_system_get_apps(skeleton->system);
    GList *match_apps = kiran_menu_search_by_keyword(skeleton->search, keyword, ignore_case, apps);
    g_list_free_full(apps, g_object_unref);
    return match_apps;
}

static gboolean kiran_menu_skeleton_add_favorite_app(KiranMenuBased *self,
                                                     const char *desktop_id)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), FALSE);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    return kiran_menu_favorite_add_app(skeleton->favorite, desktop_id);
}

static gboolean kiran_menu_skeleton_del_favorite_app(KiranMenuBased *self,
                                                     const char *desktop_id)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), FALSE);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    return kiran_menu_favorite_del_app(skeleton->favorite, desktop_id);
}

static KiranApp *kiran_menu_skeleton_lookup_favorite_app(KiranMenuBased *self,
                                                         const char *desktop_id)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), FALSE);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    gboolean exist = kiran_menu_favorite_find_app(skeleton->favorite, desktop_id);

    if (exist)
    {
        KiranMenuApp *app = kiran_menu_system_lookup_app(skeleton->system, desktop_id);
        if (app)
        {
            return g_object_ref(KIRAN_APP(app));
        }
    }
    return NULL;
}

static GList *kiran_menu_skeleton_get_favorite_apps(KiranMenuBased *self)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    GList *desktop_ids =
        kiran_menu_favorite_get_favorite_apps(skeleton->favorite);
    GList *favorite_apps = trans_ids_to_apps(skeleton, desktop_ids);

    g_list_free_full(desktop_ids, (GDestroyNotify)g_free);

    return favorite_apps;
}

static gboolean kiran_menu_skeleton_add_category_app(KiranMenuBased *self,
                                                     const char *category_name,
                                                     const char *desktop_id)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), FALSE);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    KiranMenuApp *menu_app =
        kiran_menu_system_lookup_app(skeleton->system, desktop_id);
    return kiran_menu_category_add_app(skeleton->category, category_name, menu_app);
}

gboolean kiran_menu_skeleton_del_category_app(KiranMenuBased *self,
                                              const char *category_name,
                                              const char *desktop_id)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), FALSE);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    KiranMenuApp *menu_app =
        kiran_menu_system_lookup_app(skeleton->system, desktop_id);
    return kiran_menu_category_del_app(skeleton->category, category_name, menu_app);
}

GList *kiran_menu_skeleton_get_category_names(KiranMenuBased *self)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    return kiran_menu_category_get_names(skeleton->category);
}

GList *kiran_menu_skeleton_get_category_apps(KiranMenuBased *self,
                                             const char *category_name)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);

    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    GList *desktop_ids =
        kiran_menu_category_get_apps(skeleton->category, category_name);

    GList *apps = trans_ids_to_apps(skeleton, desktop_ids);

    g_list_free_full(desktop_ids, (GDestroyNotify)g_free);

    return apps;
}

static void destory_apps_func(gpointer data)
{
    GList *apps = (GList *)data;
    g_list_free_full(apps, g_object_unref);
}

GHashTable *kiran_menu_skeleton_get_all_category_apps(KiranMenuBased *self)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    GHashTable *all =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, destory_apps_func);
    GList *categorys = kiran_menu_category_get_names(skeleton->category);
    for (GList *l = categorys; l != NULL; l = l->next)
    {
        gchar *category = l->data;
        GList *apps = kiran_menu_skeleton_get_category_apps(self, category);
        g_hash_table_insert(all, g_strdup(category), apps);
    }
    g_list_free_full(categorys, (GDestroyNotify)g_free);
    return all;
}

GList *kiran_menu_skeleton_get_nfrequent_apps(KiranMenuBased *self,
                                              gint top_n)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    GList *desktop_ids =
        kiran_menu_usage_get_nfrequent_apps(skeleton->usage, top_n);
    GList *apps = trans_ids_to_apps(skeleton, desktop_ids);

    g_list_free_full(desktop_ids, (GDestroyNotify)g_free);

    return apps;
}

void kiran_menu_skeleton_reset_frequent_apps(KiranMenuBased *self)
{
    g_return_if_fail(KIRAN_IS_MENU_SKELETON(self));
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);
    kiran_menu_usage_reset(skeleton->usage);
}

GList *kiran_menu_skeleton_get_nnew_apps(KiranMenuBased *self, gint top_n)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    GList *desktop_ids = kiran_menu_system_get_nnew_apps(skeleton->system, top_n);
    GList *apps = trans_ids_to_apps(skeleton, desktop_ids);

    g_list_free_full(desktop_ids, (GDestroyNotify)g_free);
    return apps;
}

GList *kiran_menu_skeleton_get_all_sorted_apps(KiranMenuBased *self)
{
    g_return_val_if_fail(KIRAN_IS_MENU_SKELETON(self), NULL);
    KiranMenuSkeleton *skeleton = KIRAN_MENU_SKELETON(self);

    GList *desktop_ids = kiran_menu_system_get_all_sorted_apps(skeleton->system);
    GList *apps = trans_ids_to_apps(skeleton, desktop_ids);

    g_list_free_full(desktop_ids, (GDestroyNotify)g_free);
    return apps;
}

KiranMenuUnit *kiran_menu_skeleton_get_unit(KiranMenuSkeleton *self, KiranMenuUnitType unit_type)
{
    switch (unit_type)
    {
        case KIRAN_MENU_TYPE_CATEGORY:
            return KIRAN_MENU_UNIT(self->category);
        case KIRAN_MENU_TYPE_FAVORITE:
            return KIRAN_MENU_UNIT(self->favorite);
        case KIRAN_MENU_TYPE_SEARCH:
            return KIRAN_MENU_UNIT(self->search);
        case KIRAN_MENU_TYPE_SYSTEM:
            return KIRAN_MENU_UNIT(self->system);
        case KIRAN_MENU_TYPE_USAGE:
            return KIRAN_MENU_UNIT(self->usage);
        default:
            return NULL;
    }
}

static void kiran_menu_based_interface_init(KiranMenuBasedInterface *iface)
{
    iface->impl_search_app = kiran_menu_skeleton_search_app;

    iface->impl_add_favorite_app = kiran_menu_skeleton_add_favorite_app;
    iface->impl_del_favorite_app = kiran_menu_skeleton_del_favorite_app;
    iface->impl_lookup_favorite_app = kiran_menu_skeleton_lookup_favorite_app;
    iface->impl_get_favorite_apps = kiran_menu_skeleton_get_favorite_apps;

    iface->impl_add_category_app = kiran_menu_skeleton_add_category_app;
    iface->impl_del_category_app = kiran_menu_skeleton_del_category_app;
    iface->impl_get_category_names = kiran_menu_skeleton_get_category_names;
    iface->impl_get_category_apps = kiran_menu_skeleton_get_category_apps;
    iface->impl_get_all_category_apps = kiran_menu_skeleton_get_all_category_apps;

    iface->impl_get_nfrequent_apps = kiran_menu_skeleton_get_nfrequent_apps;
    iface->impl_reset_frequent_apps = kiran_menu_skeleton_reset_frequent_apps;

    iface->impl_get_nnew_apps = kiran_menu_skeleton_get_nnew_apps;

    iface->impl_get_all_sorted_apps = kiran_menu_skeleton_get_all_sorted_apps;
}

G_DEFINE_TYPE_WITH_CODE(KiranMenuSkeleton, kiran_menu_skeleton, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(KIRAN_TYPE_MENU_BASED,
                                              kiran_menu_based_interface_init))

static void kiran_menu_skeleton_dispose(GObject *object)
{
    G_OBJECT_CLASS(kiran_menu_skeleton_parent_class)->dispose(object);
}

static void flush_menu_skeleton(GAppInfoMonitor *gappinfomonitor,
                                gpointer user_data)
{
    KiranMenuSkeleton *self = KIRAN_MENU_SKELETON(user_data);

    kiran_menu_unit_flush(KIRAN_MENU_UNIT(self->system), NULL);

    GList *apps = kiran_menu_system_get_apps(self->system);
    kiran_menu_unit_flush(KIRAN_MENU_UNIT(self->favorite), apps);
    kiran_menu_unit_flush(KIRAN_MENU_UNIT(self->category), apps);
    g_list_free_full(apps, g_object_unref);

    g_signal_emit(self, signals[INSTALLED_CHANGED], 0, NULL);
}

static void kiran_menu_skeleton_init(KiranMenuSkeleton *self)
{
    self->system = kiran_menu_system_get_new();
    self->usage = kiran_menu_usage_get_new();
    self->favorite = kiran_menu_favorite_get_new();
    self->search = kiran_menu_search_get_new();
    self->category = kiran_menu_category_get_new();

    GAppInfoMonitor *monitor = g_app_info_monitor_get();
    g_signal_connect(monitor, "changed", G_CALLBACK(flush_menu_skeleton), self);
    flush_menu_skeleton(monitor, self);
}

static void kiran_menu_skeleton_class_init(KiranMenuSkeletonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = kiran_menu_skeleton_dispose;

    signals[INSTALLED_CHANGED] = g_signal_new("installed-changed", KIRAN_TYPE_MENU_SKELETON,
                                              G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}