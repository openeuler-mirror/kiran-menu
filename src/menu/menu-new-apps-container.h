/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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
    std::vector<MenuAppItem*> hidden_items;
};

#endif // __MENU_NEW_APPS_CONTAINER_H__