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