/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include "lib/menu-unit.h"

namespace Kiran
{
class MenuFavorite : public MenuUnit
{
public:
    MenuFavorite();
    virtual ~MenuFavorite();

    virtual void init();

    virtual void flush(const AppVec &apps);

    bool add_app(const std::string &desktop_id);

    bool del_app(const std::string &desktop_id);

    bool find_app(const std::string &desktop_id);

    std::vector<std::string> get_favorite_apps();

    //signal accessor:
    sigc::signal<void, std::vector<std::string>> &signal_app_added() { return this->app_added_; }
    sigc::signal<void, std::vector<std::string>> &signal_app_deleted() { return this->app_deleted_; }

private:
    void app_changed(const Glib::ustring &key);

protected:
    sigc::signal<void, std::vector<std::string>> app_added_;
    sigc::signal<void, std::vector<std::string>> app_deleted_;

private:
    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> favorite_apps_;
};

}  // namespace Kiran