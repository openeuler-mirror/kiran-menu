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
    if (user)
    {
        delete user;
    }
}

bool MenuUserInfo::load()
{
    if (load_state != INFO_STATE_NOT_LOAD)
        return true;

    auto manager = kiran_accounts_manager_get_default();

    user = kiran_accounts_manager_get_user_by_id(manager, uid);
    g_signal_connect_swapped(user, "changed", G_CALLBACK(&MenuUserInfo::on_changed), this);
    if (!kiran_accounts_user_get_is_loaded(user))
    {
        load_state = INFO_STATE_LOADING;
        handler_id = g_signal_connect_swapped(user, "loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    }
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
    return kiran_accounts_user_get_name(user);
}

const char *MenuUserInfo::get_iconfile() const
{
    return kiran_accounts_user_get_icon_file(user);
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
