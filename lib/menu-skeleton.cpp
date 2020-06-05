/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 19:59:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 20:24:00
 * @Description  : 开始菜单类
 * @FilePath     : /kiran-menu-2.0/lib/menu-skeleton.cpp
 */

#include "lib/menu-skeleton.h"

namespace Kiran
{
static void flush_menu_skeleton(GAppInfoMonitor *gappinfomonitor,
                                gpointer user_data)
{
    MenuSkeleton *self = (MenuSkeleton *)(user_data);
    self->flush();
}

MenuSkeleton::MenuSkeleton() : system(new MenuSystem()),
                               usage(new MenuUsage()),
                               favorite(new MenuFavorite()),
                               search(new MenuSearch()),
                               category(new MenuCategory())
{
    GAppInfoMonitor *monitor = g_app_info_monitor_get();
    g_signal_connect(monitor, "changed", G_CALLBACK(flush_menu_skeleton), this);
    flush_menu_skeleton(monitor, this);

    this->system->signal_app_installed().connect(sigc::mem_fun(this, &MenuSkeleton::app_installed));
    this->system->signal_app_uninstalled().connect(sigc::mem_fun(this, &MenuSkeleton::app_uninstalled));
    this->system->signal_new_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::new_app_changed));

    this->usage->signal_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::frequent_usage_app_changed));

    this->favorite->signal_app_added().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_added));
    this->favorite->signal_app_deleted().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_deleted));
}

MenuSkeleton::~MenuSkeleton()
{
}

MenuSkeleton *MenuSkeleton::instance_ = nullptr;
MenuSkeleton *MenuSkeleton::get_instance()
{
    if (!instance_)
    {
        instance_ = new MenuSkeleton();
    }
    return instance_;
}

AppVec MenuSkeleton::search_app(const std::string &keyword, bool ignore_case)
{
    auto apps = this->system->get_apps();
    auto match_apps = this->search->search_by_keyword(keyword, ignore_case, apps);
    return match_apps;
}

gboolean MenuSkeleton::add_favorite_app(const std::string &desktop_id)
{
    return this->favorite->add_app(desktop_id);
}

gboolean MenuSkeleton::del_favorite_app(const std::string &desktop_id)
{
    return this->favorite->del_app(desktop_id);
}

std::shared_ptr<App> MenuSkeleton::lookup_favorite_app(const std::string &desktop_id)
{
    gboolean exist = this->favorite->find_app(desktop_id);

    if (exist)
    {
        return this->system->lookup_app(desktop_id);
    }
    return nullptr;
}

AppVec MenuSkeleton::get_favorite_apps()
{
    auto desktop_ids = this->favorite->get_favorite_apps();
    return trans_ids_to_apps(desktop_ids);
}

gboolean MenuSkeleton::add_category_app(const std::string &category_name, const std::string &desktop_id)
{
    auto app = this->system->lookup_app(desktop_id);
    return this->category->add_app(category_name, app);
}

gboolean MenuSkeleton::del_category_app(const std::string &category_name, const std::string &desktop_id)
{
    auto app = this->system->lookup_app(desktop_id);
    return this->category->del_app(category_name, app);
}

std::vector<std::string> MenuSkeleton::get_category_names()
{
    return this->category->get_names();
}

AppVec MenuSkeleton::get_category_apps(const std::string &category_name)
{
    auto desktop_ids = this->category->get_apps(category_name);
    return trans_ids_to_apps(desktop_ids);
}

std::map<std::string, AppVec> MenuSkeleton::get_all_category_apps()
{
    std::map<std::string, AppVec> categories;
    auto category_names = this->category->get_names();

    for (int i = 0; i < category_names.size(); ++i)
    {
        categories[category_names[i]] = get_category_apps(category_names[i]);
    }
    return categories;
}

AppVec MenuSkeleton::get_nfrequent_apps(gint top_n)
{
    auto desktop_ids = this->usage->get_nfrequent_apps(top_n);
    return trans_ids_to_apps(desktop_ids);
}

void MenuSkeleton::reset_frequent_apps()
{
    this->usage->reset();
}

AppVec MenuSkeleton::get_nnew_apps(gint top_n)
{
    auto desktop_ids = this->system->get_nnew_apps(top_n);
    return trans_ids_to_apps(desktop_ids);
}

AppVec MenuSkeleton::get_all_sorted_apps()
{
    auto desktop_ids = this->system->get_all_sorted_apps();
    return trans_ids_to_apps(desktop_ids);
}

std::shared_ptr<MenuUnit> MenuSkeleton::get_unit(MenuUnitType unit_type)
{
    switch (unit_type)
    {
        case MenuUnitType::KIRAN_MENU_TYPE_CATEGORY:
            return this->category;
        case MenuUnitType::KIRAN_MENU_TYPE_FAVORITE:
            return this->favorite;
        case MenuUnitType::KIRAN_MENU_TYPE_SEARCH:
            return this->search;
        case MenuUnitType::KIRAN_MENU_TYPE_SYSTEM:
            return this->system;
        case MenuUnitType::KIRAN_MENU_TYPE_USAGE:
            return this->usage;
        default:
            return nullptr;
    }
}

void MenuSkeleton::flush()
{
    this->system->flush(AppVec());
    auto apps = this->system->get_apps();
    this->favorite->flush(apps);
    this->category->flush(apps);

    this->signal_app_changed_.emit();
}

AppVec MenuSkeleton::trans_ids_to_apps(const std::vector<std::string> &desktop_ids)
{
    std::vector<std::shared_ptr<App>> apps;

    for (int i = 0; i < desktop_ids.size(); ++i)
    {
        auto app = this->system->lookup_app(desktop_ids[i]);
        if (app)
        {
            apps.push_back(app);
        }
    }
    return apps;
}

void MenuSkeleton::app_installed(AppVec apps)
{
    this->signal_app_installed_.emit(apps);
}

void MenuSkeleton::app_uninstalled(AppVec apps)
{
    this->signal_app_uninstalled_.emit(apps);
}

void MenuSkeleton::new_app_changed()
{
    this->signal_new_app_changed_.emit();
}

void MenuSkeleton::frequent_usage_app_changed()
{
    this->signal_frequent_usage_app_changed_.emit();
}

void MenuSkeleton::favorite_app_added(std::vector<std::string> desktop_ids)
{
    auto apps = trans_ids_to_apps(desktop_ids);
    this->signal_favorite_app_added_.emit(apps);
}

void MenuSkeleton::favorite_app_deleted(std::vector<std::string> desktop_ids)
{
    auto apps = trans_ids_to_apps(desktop_ids);
    this->signal_favorite_app_deleted_.emit(apps);
}

}  // namespace Kiran