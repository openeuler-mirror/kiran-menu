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

#include "lib/menu-favorite.h"

#include <set>

#include "lib/app.h"
#include "lib/common.h"

namespace Kiran
{
MenuFavorite::MenuFavorite()
{
    this->settings_ = Gio::Settings::create(STARTMENU_SCHEMA);
    this->favorite_apps_ = read_as_to_list_quark(this->settings_, STARTMENU_KEY_FAVORITE_APPS);
}

MenuFavorite::~MenuFavorite()
{
}

void MenuFavorite::init()
{
    this->settings_->signal_changed(STARTMENU_KEY_FAVORITE_APPS).connect(sigc::mem_fun(this, &MenuFavorite::app_changed));
}

void MenuFavorite::flush(const AppVec &apps)
{
    std::set<int32_t> app_set;

    for (int i = 0; i < (int)apps.size(); ++i)
    {
        auto &app = apps[i];
        auto &desktop_id = app->get_desktop_id();
        Glib::Quark quark(desktop_id);
        app_set.insert(quark.id());
    }

    std::vector<std::string> delete_apps;

    auto iter = std::remove_if(this->favorite_apps_.begin(), this->favorite_apps_.end(), [this, &delete_apps, &app_set](int32_t elem) -> bool
                               {
                                   if (app_set.find(elem) == app_set.end())
                                   {
                                       Glib::QueryQuark query_quark((GQuark)elem);
                                       Glib::ustring desktop_id = query_quark;
                                       delete_apps.push_back(desktop_id.raw());
                                       return true;
                                   }
                                   return false;
                               });

    if (iter != this->favorite_apps_.end())
    {
        this->favorite_apps_.erase(iter, this->favorite_apps_.end());
        write_list_quark_to_as(this->settings_, STARTMENU_KEY_FAVORITE_APPS, this->favorite_apps_);
        this->app_deleted_.emit(delete_apps);
    }
}

bool MenuFavorite::add_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps_.begin(), this->favorite_apps_.end(), quark.id());
    if (iter == this->favorite_apps_.end())
    {
        this->favorite_apps_.push_back(quark.id());
        std::vector<std::string> add_apps = {desktop_id};
        this->app_added_.emit(add_apps);
        return write_list_quark_to_as(this->settings_, STARTMENU_KEY_FAVORITE_APPS, this->favorite_apps_);
    }
    return FALSE;
}

bool MenuFavorite::del_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps_.begin(), this->favorite_apps_.end(), quark.id());

    if (iter != this->favorite_apps_.end())
    {
        this->favorite_apps_.erase(iter);
        std::vector<std::string> delete_apps = {desktop_id};
        this->app_deleted_.emit(delete_apps);
        return write_list_quark_to_as(this->settings_, STARTMENU_KEY_FAVORITE_APPS, this->favorite_apps_);
    }
    return FALSE;
}

bool MenuFavorite::find_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps_.begin(), this->favorite_apps_.end(), quark.id());

    return (iter != this->favorite_apps_.end());
}

std::vector<std::string> MenuFavorite::get_favorite_apps()
{
    std::vector<std::string> apps;
    for (auto iter = this->favorite_apps_.begin(); iter != this->favorite_apps_.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)*iter);
        Glib::ustring desktop_id = query_quark;
        apps.push_back(desktop_id.raw());
    }
    return apps;
}

void MenuFavorite::app_changed(const Glib::ustring &key)
{
    auto new_favorite_apps = read_as_to_list_quark(this->settings_, STARTMENU_KEY_FAVORITE_APPS);

    std::vector<std::string> add_apps;
    std::vector<std::string> delete_apps;

    for (auto iter = this->favorite_apps_.begin(); iter != this->favorite_apps_.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(new_favorite_apps.begin(), new_favorite_apps.end(), value) == new_favorite_apps.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            delete_apps.push_back(desktop_id);
        }
    }

    for (auto iter = new_favorite_apps.begin(); iter != new_favorite_apps.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(this->favorite_apps_.begin(), this->favorite_apps_.end(), value) == this->favorite_apps_.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            add_apps.push_back(desktop_id);
        }
    }

    this->favorite_apps_ = new_favorite_apps;

    if (delete_apps.size() > 0)
    {
        this->app_deleted_.emit(delete_apps);
    }

    if (add_apps.size() > 0)
    {
        this->app_added_.emit(add_apps);
    }
}

}  // namespace Kiran