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

#ifndef MENU_APP_CONTEXT_MENU_H
#define MENU_APP_CONTEXT_MENU_H

#include "app.h"
#include "kiran-opacity-menu.h"

class TasklistAppContextMenu : public KiranOpacityMenu
{
public:
    /**
     * @brief TasklistAppContextMenu
     * @param app  右键菜单关联的应用
     */
    TasklistAppContextMenu(const std::shared_ptr<Kiran::App> &app);

    /**
     * @brief refresh 更新右键菜单内容
     */
    void refresh();

private:
    std::weak_ptr<Kiran::App> app;
};

#endif  // MENU_APP_CONTEXT_MENU_H
