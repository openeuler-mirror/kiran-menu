/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 21:00:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:14:44
 * @Description  : 开始菜单类，继承KiranMenuBased接口类。
 * @FilePath     : /kiran-menu-2.0/lib/menu-skeleton.h
 */
#pragma once

#include "lib/menu-based.h"
#include "lib/menu-category.h"
#include "lib/menu-favorite.h"
#include "lib/menu-search.h"
#include "lib/menu-system.h"
#include "lib/menu-usage.h"

namespace Kiran
{
enum class MenuUnitType
{
    KIRAN_MENU_TYPE_CATEGORY,
    KIRAN_MENU_TYPE_FAVORITE,
    KIRAN_MENU_TYPE_SEARCH,
    KIRAN_MENU_TYPE_SYSTEM,
    KIRAN_MENU_TYPE_USAGE,
};

class MenuSkeleton : public MenuBased
{
   public:
    MenuSkeleton();
    virtual ~MenuSkeleton();

    static MenuSkeleton *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

    virtual AppVec search_app(const std::string &keyword, bool ignore_case = false);

    virtual bool add_favorite_app(const std::string &desktop_id);

    virtual bool del_favorite_app(const std::string &desktop_id);

    virtual std::shared_ptr<App> lookup_favorite_app(const std::string &desktop_id);

    virtual AppVec get_favorite_apps();

    virtual bool add_category_app(const std::string &category_name, const std::string &desktop_id);

    virtual bool del_category_app(const std::string &category_name, const std::string &desktop_id);

    virtual std::vector<std::string> get_category_names();

    virtual AppVec get_category_apps(const std::string &category_name);

    virtual std::map<std::string, AppVec> get_all_category_apps();

    virtual AppVec get_nfrequent_apps(gint top_n);

    virtual void reset_frequent_apps();

    virtual AppVec get_nnew_apps(gint top_n);

    virtual AppVec get_all_sorted_apps();

    std::shared_ptr<MenuUnit> get_unit(MenuUnitType unit_type);

    void flush();

    //signal accessor:
    sigc::signal<void> &signal_app_changed() { return this->app_changed_; }
    sigc::signal<void, AppVec> &signal_app_installed() { return this->app_installed_; }
    sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->app_uninstalled_; }
    sigc::signal<void> &signal_new_app_changed() { return this->new_app_changed_; }
    sigc::signal<void, AppVec> &signal_favorite_app_added() { return this->favorite_app_added_; }
    sigc::signal<void, AppVec> &signal_favorite_app_deleted() { return this->favorite_app_deleted_; }
    sigc::signal<void> &signal_frequent_usage_app_changed() { return this->frequent_usage_app_changed_; }

   private:
    AppVec trans_ids_to_apps(const std::vector<std::string> &desktop_ids);

    void app_installed(AppVec apps);
    void app_uninstalled(AppVec apps);
    void new_app_changed();
    void frequent_usage_app_changed();
    void favorite_app_added(std::vector<std::string> desktop_ids);
    void favorite_app_deleted(std::vector<std::string> desktop_ids);

   protected:
    sigc::signal<void> app_changed_;
    sigc::signal<void, AppVec> app_installed_;
    sigc::signal<void, AppVec> app_uninstalled_;
    sigc::signal<void> new_app_changed_;
    sigc::signal<void, AppVec> favorite_app_added_;
    sigc::signal<void, AppVec> favorite_app_deleted_;
    sigc::signal<void> frequent_usage_app_changed_;

   private:
    static MenuSkeleton *instance_;

    std::shared_ptr<MenuSystem> system_;
    std::shared_ptr<MenuUsage> usage_;
    std::shared_ptr<MenuSearch> search_;
    std::shared_ptr<MenuFavorite> favorite_;
    std::shared_ptr<MenuCategory> category_;
};

}  // namespace Kiran