/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include "lib/app-manager.h"
#include "lib/menu-category.h"
#include "lib/menu-favorite.h"
#include "lib/menu-new.h"
#include "lib/menu-search.h"
#include "lib/menu-usage.h"

namespace Kiran
{
enum class MenuUnitType
{
    KIRAN_MENU_TYPE_CATEGORY,
    KIRAN_MENU_TYPE_FAVORITE,
    KIRAN_MENU_TYPE_SEARCH,
    KIRAN_MENU_TYPE_NEW,
    KIRAN_MENU_TYPE_USAGE,
};

class MenuSkeleton
{
public:
    MenuSkeleton() = delete;

    MenuSkeleton(AppManager *app_manager);
    virtual ~MenuSkeleton();

    static MenuSkeleton *get_instance() { return instance_; };

    static void global_init(AppManager *app_manager);

    static void global_deinit();

    void init();

    // 通过keyword字段进行检索, 会跟desktop文件的name/localename/comment字段进行字符串匹配, 最重返回所有匹配成功的App。如果ignore_case设置为true，则忽略大小写匹配。
    AppVec search_app(const std::string &keyword, bool ignore_case = false, AppKind kind = AppKind::NORMAL);

    // 将desktop_id加入收藏列表，如果desktop_id不合法, 或者已经在收藏列表中, 则返回false, 否则返回true.
    bool add_favorite_app(const std::string &desktop_id);

    // 从收藏列表删除desktop_id，如果desktop_id不在收藏列表中, 则返回false, 否则返回true
    bool del_favorite_app(const std::string &desktop_id);

    // 查询desktop_id是否在收藏列表中，如果未查询到，则返回空指针
    std::shared_ptr<App> lookup_favorite_app(const std::string &desktop_id);

    // 获取收藏列表
    AppVec get_favorite_apps();

    // 将desktop_id添加到category分类中，如果desktop_id不存在或者添加分类错误, 则返回false, 否则返回true
    bool add_category_app(const std::string &category_name, const std::string &desktop_id);

    // 将desktop_id从category分类中删除，如果desktop_id不存在或者删除分类错误, 则返回false, 否则返回true
    bool del_category_app(const std::string &category_name, const std::string &desktop_id);

    // 获取所有分类的名字
    std::vector<std::string> get_category_names();

    // 获取category分类中的所有App.
    AppVec get_category_apps(const std::string &category_name);

    // 获取所有分类的App.
    std::map<std::string, AppVec> get_all_category_apps();

    // 获取使用频率最高的top_n个app，如果top_n超过所有app的数量或者等于-1，则返回所有App。
    AppVec get_nfrequent_apps(gint top_n);

    // 重置频繁使用的APP列表.该操作会将所有APP的分数清0
    void reset_frequent_apps();

    // 获取最新安装的top_n个app, 返回这些app的App对象。如果top_n超过所有app的数量或者等于-1,则返回所有App
    AppVec get_nnew_apps(gint top_n);

    // 获取所有已注册且可在当前系统显示的desktop_id列表，该列表已通过desktop文件的name字段进行排序
    AppVec get_all_sorted_apps();

    std::shared_ptr<MenuUnit> get_unit(MenuUnitType unit_type);

    //signal accessor:
    // desktop应用列表发生变化信号，与AppManager::signal_desktop_app_changed相同
    sigc::signal<void> &signal_app_changed() { return this->app_changed_; }

    // App安装时的信号，这里会过滤掉should_show返回false的应用
    sigc::signal<void, AppVec> &signal_app_installed() { return this->app_installed_; }

    // App卸载时的信号，这里会过滤掉should_show返回false的应用
    sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->app_uninstalled_; }

    // 新安装应用列表发生变化的信号
    sigc::signal<void> &signal_new_app_changed() { return this->new_app_changed_; }

    // 收藏app被添加信号
    sigc::signal<void, AppVec> &signal_favorite_app_added() { return this->favorite_app_added_; }

    // 收藏app被移除信号
    sigc::signal<void, AppVec> &signal_favorite_app_deleted() { return this->favorite_app_deleted_; }

    // 常用app列表变化信号
    sigc::signal<void> &signal_frequent_usage_app_changed() { return this->frequent_usage_app_changed_; }

private:
    AppVec trans_ids_to_apps(const std::vector<std::string> &desktop_ids);

    void desktop_app_changed();
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

    AppManager *app_manager_;

    std::shared_ptr<MenuUsage> usage_;
    std::shared_ptr<MenuSearch> search_;
    std::shared_ptr<MenuFavorite> favorite_;
    std::shared_ptr<MenuCategory> category_;
    std::shared_ptr<MenuNew> new_;
};

}  // namespace Kiran