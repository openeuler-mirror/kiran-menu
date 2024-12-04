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
    explicit TasklistAppContextMenu(const std::shared_ptr<Kiran::App> &app);

    /**
     * @brief refresh 更新右键菜单内容
     */
    void refresh();

private:
    std::weak_ptr<Kiran::App> app;
    Glib::RefPtr<Gio::Settings> settings_;
};

#endif  // MENU_APP_CONTEXT_MENU_H
