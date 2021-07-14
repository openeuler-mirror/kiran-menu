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

#include "menu-user-info.h"
#include <iostream>
#include "lib/base.h"

enum
{
    INFO_STATE_NOT_LOAD,
    INFO_STATE_LOADING,
    INFO_STATE_LOADED
};

MenuUserInfo::MenuUserInfo(uid_t id) : uid(id),
                                       user(nullptr),
                                       load_state(INFO_STATE_NOT_LOAD)
{
}

MenuUserInfo::~MenuUserInfo()
{
}

bool MenuUserInfo::load()
{
    if (load_state != INFO_STATE_NOT_LOAD)
        return true;

#ifdef BUILD_WITH_KIRANACCOUNTS
    auto manager = kiran_accounts_manager_get_default();

    user = kiran_accounts_manager_get_user_by_id(manager, uid);
    g_signal_connect_swapped(user, "changed", G_CALLBACK(&MenuUserInfo::on_changed), this);
    if (!kiran_accounts_user_get_is_loaded(user))
    {
        load_state = INFO_STATE_LOADING;
        handler_id = g_signal_connect_swapped(user, "loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    }
#else
    auto manager = act_user_manager_get_default();

    user = act_user_manager_get_user_by_id(manager, uid);
    g_signal_connect_swapped(user, "changed", G_CALLBACK(&MenuUserInfo::on_changed), this);
    if (!act_user_is_loaded(user))
    {
        load_state = INFO_STATE_LOADING;
        handler_id = g_signal_connect_swapped(user, "notify::is-loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    }
#endif
    else
        on_loaded(this);

    return true;
}

bool MenuUserInfo::is_ready() const
{
    return load_state == INFO_STATE_LOADED;
}

void MenuUserInfo::on_loaded(MenuUserInfo *info)
{
    info->load_state = INFO_STATE_LOADED;
    g_signal_handler_disconnect(info->user, info->handler_id);
    info->signal_ready().emit();
}

const char *MenuUserInfo::get_username() const
{
#ifdef BUILD_WITH_KIRANACCOUNTS
    return kiran_accounts_user_get_name(user);
#else
    return act_user_get_user_name(user);
#endif
}

const char *MenuUserInfo::get_iconfile() const
{
#ifdef BUILD_WITH_KIRANACCOUNTS
    return kiran_accounts_user_get_icon_file(user);
#else
    return act_user_get_icon_file(user);
#endif
}

sigc::signal<void> MenuUserInfo::signal_ready()
{
    return m_signal_ready;
}

sigc::signal<void> MenuUserInfo::signal_changed()
{
    return m_signal_changed;
}

void MenuUserInfo::on_changed(MenuUserInfo *info)
{
    KLOG_INFO("got changed signal for user information");
    info->signal_changed().emit();
}
