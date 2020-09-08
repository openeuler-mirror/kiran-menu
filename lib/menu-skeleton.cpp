/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 19:59:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 15:58:07
 * @Description  : 开始菜单类
 * @FilePath     : /kiran-menu-2.0/lib/menu-skeleton.cpp
 */

#include "lib/menu-skeleton.h"

#include "lib/app-manager.h"
#include "lib/log.h"
#include "lib/window-manager.h"

namespace Kiran
{
MenuSkeleton::MenuSkeleton(AppManager *app_manager) : app_manager_(app_manager),
                                                      usage_(new MenuUsage()),
                                                      favorite_(new MenuFavorite()),
                                                      search_(new MenuSearch()),
                                                      category_(new MenuCategory()),
                                                      new_(new MenuNew())
{
}

MenuSkeleton::~MenuSkeleton()
{
}

MenuSkeleton *MenuSkeleton::instance_ = nullptr;
void MenuSkeleton::global_init(AppManager *app_manager)
{
    instance_ = new MenuSkeleton(app_manager);
    instance_->init();
}

void MenuSkeleton::global_deinit()
{
    delete instance_;
}

void MenuSkeleton::init()
{
    this->usage_->init();
    this->favorite_->init();
    this->search_->init();
    this->category_->init();
    this->new_->init();

    this->desktop_app_changed();

    app_manager_->signal_desktop_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::desktop_app_changed));
    app_manager_->signal_app_installed().connect(sigc::mem_fun(this, &MenuSkeleton::app_installed));
    app_manager_->signal_app_uninstalled().connect(sigc::mem_fun(this, &MenuSkeleton::app_uninstalled));

    this->new_->signal_new_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::new_app_changed));
    this->usage_->signal_app_changed().connect(sigc::mem_fun(this, &MenuSkeleton::frequent_usage_app_changed));
    this->favorite_->signal_app_added().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_added));
    this->favorite_->signal_app_deleted().connect(sigc::mem_fun(this, &MenuSkeleton::favorite_app_deleted));
}

AppVec MenuSkeleton::search_app(const std::string &keyword, bool ignore_case)
{
    auto apps = AppManager::get_instance()->get_should_show_apps();
    auto match_apps = this->search_->search_by_keyword(keyword, ignore_case, apps);
    return match_apps;
}

#define RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, ret)                       \
    {                                                                           \
        auto app = app_manager_->lookup_app(desktop_id);                        \
        if (!app)                                                               \
        {                                                                       \
            LOG_WARNING("not found the %s in AppManager.", desktop_id.c_str()); \
            return ret;                                                         \
        }                                                                       \
        if (!(app->should_show()))                                              \
        {                                                                       \
            LOG_WARNING("the %s cannot show in menu.", desktop_id.c_str());     \
            return ret;                                                         \
        }                                                                       \
    }

bool MenuSkeleton::add_favorite_app(const std::string &desktop_id)
{
    RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, false);

    return this->favorite_->add_app(desktop_id);
}

bool MenuSkeleton::del_favorite_app(const std::string &desktop_id)
{
    RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, false);

    return this->favorite_->del_app(desktop_id);
}

std::shared_ptr<App> MenuSkeleton::lookup_favorite_app(const std::string &desktop_id)
{
    RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, nullptr);

    bool exist = this->favorite_->find_app(desktop_id);

    if (exist)
    {
        return AppManager::get_instance()->lookup_app(desktop_id);
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
    RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, false);

    auto app = AppManager::get_instance()->lookup_app(desktop_id);
    return this->category_->add_app(category_name, app);
}

bool MenuSkeleton::del_category_app(const std::string &category_name, const std::string &desktop_id)
{
    RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, false);

    auto app = AppManager::get_instance()->lookup_app(desktop_id);
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
    auto desktop_ids = this->new_->get_nnew_apps(top_n);
    return trans_ids_to_apps(desktop_ids);
}

AppVec MenuSkeleton::get_all_sorted_apps()
{
    auto desktop_ids = AppManager::get_instance()->get_all_sorted_apps();
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
    case MenuUnitType::KIRAN_MENU_TYPE_NEW:
        return this->new_;
    case MenuUnitType::KIRAN_MENU_TYPE_USAGE:
        return this->usage_;
    default:
        return nullptr;
    }
}

AppVec MenuSkeleton::trans_ids_to_apps(const std::vector<std::string> &desktop_ids)
{
    std::vector<std::shared_ptr<App>> apps;

    for (int i = 0; i < desktop_ids.size(); ++i)
    {
        auto app = AppManager::get_instance()->lookup_app(desktop_ids[i]);
        if (app && app->should_show())
        {
            apps.push_back(app);
        }
    }
    return apps;
}

void MenuSkeleton::desktop_app_changed()
{
    auto apps = app_manager_->get_should_show_apps();
    this->favorite_->flush(apps);
    this->category_->flush(apps);

    this->app_changed_.emit();
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