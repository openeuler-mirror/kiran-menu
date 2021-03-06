/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 19:59:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:15:51
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

MenuSkeleton::MenuSkeleton() : system_(new MenuSystem()),
                               usage_(new MenuUsage()),
                               favorite_(new MenuFavorite()),
                               search_(new MenuSearch()),
                               category_(new MenuCategory())
{
}

MenuSkeleton::~MenuSkeleton()
{
}

MenuSkeleton *MenuSkeleton::instance_ = nullptr;
void MenuSkeleton::global_init()
{
    instance_ = new MenuSkeleton();
    instance_->init();
}

void MenuSkeleton::init()
{
    this->system_->init();
    this->usage_->init();
    this->favorite_->init();
    this->search_->init();
    this->category_->init();

    GAppInfoMonitor *monitor = g_app_info_monitor_get();
    g_signal_connect(monitor, "changed", G_CALLBACK(flush_menu_skeleton), this);
    flush_menu_skeleton(monitor, this);

    this->system_->signal_app_installed().connect(sigc::mem_fun(this, &MenuSkeleton::app_installed));
    this->system_->signal_app_uninstalled().connect(sigc::mem_fun(this, &MenuSkeleton::app_uninstalled));
    this->system_->signal_new_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::new_app_changed));

    this->usage_->signal_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::frequent_usage_app_changed));

    this->favorite_->signal_app_added().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_added));
    this->favorite_->signal_app_deleted().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_deleted));
}

AppVec MenuSkeleton::search_app(const std::string &keyword, bool ignore_case)
{
    auto apps = this->system_->get_apps();
    auto match_apps = this->search_->search_by_keyword(keyword, ignore_case, apps);
    return match_apps;
}

bool MenuSkeleton::add_favorite_app(const std::string &desktop_id)
{
    return this->favorite_->add_app(desktop_id);
}

bool MenuSkeleton::del_favorite_app(const std::string &desktop_id)
{
    return this->favorite_->del_app(desktop_id);
}

std::shared_ptr<App> MenuSkeleton::lookup_favorite_app(const std::string &desktop_id)
{
    bool exist = this->favorite_->find_app(desktop_id);

    if (exist)
    {
        return this->system_->lookup_app(desktop_id);
    }
    return nullptr;
}

AppVec MenuSkeleton::get_favorite_apps()
{
    auto desktop_ids = this->favorite_->get_favorite_apps();
    return trans_ids_to_apps(desktop_ids);
}

bool MenuSkeleton::add_category_app(const std::string &category_name, const std::string &desktop_id)
{
    auto app = this->system_->lookup_app(desktop_id);
    return this->category_->add_app(category_name, app);
}

bool MenuSkeleton::del_category_app(const std::string &category_name, const std::string &desktop_id)
{
    auto app = this->system_->lookup_app(desktop_id);
    return this->category_->del_app(category_name, app);
}

std::vector<std::string> MenuSkeleton::get_category_names()
{
    return this->category_->get_names();
}

AppVec MenuSkeleton::get_category_apps(const std::string &category_name)
{
    auto desktop_ids = this->category_->get_apps(category_name);
    return trans_ids_to_apps(desktop_ids);
}

std::map<std::string, AppVec> MenuSkeleton::get_all_category_apps()
{
    std::map<std::string, AppVec> categories;
    auto category_names = this->category_->get_names();

    for (int i = 0; i < category_names.size(); ++i)
    {
        categories[category_names[i]] = get_category_apps(category_names[i]);
    }
    return categories;
}

AppVec MenuSkeleton::get_nfrequent_apps(gint top_n)
{
    auto desktop_ids = this->usage_->get_nfrequent_apps(top_n);
    return trans_ids_to_apps(desktop_ids);
}

void MenuSkeleton::reset_frequent_apps()
{
    this->usage_->reset();
}

AppVec MenuSkeleton::get_nnew_apps(gint top_n)
{
    auto desktop_ids = this->system_->get_nnew_apps(top_n);
    return trans_ids_to_apps(desktop_ids);
}

AppVec MenuSkeleton::get_all_sorted_apps()
{
    auto desktop_ids = this->system_->get_all_sorted_apps();
    return trans_ids_to_apps(desktop_ids);
}

std::shared_ptr<MenuUnit> MenuSkeleton::get_unit(MenuUnitType unit_type)
{
    switch (unit_type)
    {
        case MenuUnitType::KIRAN_MENU_TYPE_CATEGORY:
            return this->category_;
        case MenuUnitType::KIRAN_MENU_TYPE_FAVORITE:
            return this->favorite_;
        case MenuUnitType::KIRAN_MENU_TYPE_SEARCH:
            return this->search_;
        case MenuUnitType::KIRAN_MENU_TYPE_SYSTEM:
            return this->system_;
        case MenuUnitType::KIRAN_MENU_TYPE_USAGE:
            return this->usage_;
        default:
            return nullptr;
    }
}

void MenuSkeleton::flush()
{
    this->system_->flush(AppVec());
    auto apps = this->system_->get_apps();
    this->favorite_->flush(apps);
    this->category_->flush(apps);

    this->app_changed_.emit();
}

AppVec MenuSkeleton::trans_ids_to_apps(const std::vector<std::string> &desktop_ids)
{
    std::vector<std::shared_ptr<App>> apps;

    for (int i = 0; i < desktop_ids.size(); ++i)
    {
        auto app = this->system_->lookup_app(desktop_ids[i]);
        if (app)
        {
            apps.push_back(app);
        }
    }
    return apps;
}

void MenuSkeleton::app_installed(AppVec apps)
{
    this->app_installed_.emit(apps);
}

void MenuSkeleton::app_uninstalled(AppVec apps)
{
    this->app_uninstalled_.emit(apps);
}

void MenuSkeleton::new_app_changed()
{
    this->new_app_changed_.emit();
}

void MenuSkeleton::frequent_usage_app_changed()
{
    this->frequent_usage_app_changed_.emit();
}

void MenuSkeleton::favorite_app_added(std::vector<std::string> desktop_ids)
{
    auto apps = trans_ids_to_apps(desktop_ids);
    this->favorite_app_added_.emit(apps);
}

void MenuSkeleton::favorite_app_deleted(std::vector<std::string> desktop_ids)
{
    auto apps = trans_ids_to_apps(desktop_ids);
    this->favorite_app_deleted_.emit(apps);
}

}  // namespace Kiran