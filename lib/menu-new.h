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

#include "lib/app.h"
#include "lib/menu-unit.h"

namespace Kiran
{
class MenuNew : public MenuUnit
{
public:
    MenuNew();
    virtual ~MenuNew();

    virtual void init();

    virtual void flush(const AppVec &apps);

    std::vector<std::string> get_nnew_apps(gint top_n);

    void remove_from_new_apps(std::shared_ptr<App> app);

    //signal accessor:
    sigc::signal<void> &signal_new_app_changed() { return this->new_app_changed_; }

private:
    void app_installed(AppVec apps);
    void app_uninstalled(AppVec apps);
    void app_action_changed(std::shared_ptr<App> app, AppAction action);
    void window_opened(std::shared_ptr<Window> window);

    void read_new_apps();
    void write_new_apps();

protected:
    sigc::signal<void> new_app_changed_;

private:
    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> new_apps_;
};

}  // namespace Kiran