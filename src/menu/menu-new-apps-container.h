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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#ifndef __MENU_NEW_APPS_CONTAINER_H__
#define __MENU_NEW_APPS_CONTAINER_H__

#include "menu-apps-container.h"

class MenuNewAppsContainer : public MenuAppsContainer
{
public:
    MenuNewAppsContainer(int max_apps_);
    bool load_applications(const Kiran::AppVec &apps);

protected:
    Gtk::ToggleButton *create_expand_button();

private:
    int max_apps;
    Gtk::Box *more_apps_box;
    Gtk::ToggleButton *expand_button;
    std::vector<MenuAppItem *> hidden_items;
};

#endif  // __MENU_NEW_APPS_CONTAINER_H__