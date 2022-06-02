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