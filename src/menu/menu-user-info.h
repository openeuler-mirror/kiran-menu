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

#ifndef MENU_USER_INFO_H
#define MENU_USER_INFO_H

#include <sigc++/sigc++.h>
#include <sys/types.h>
#include "../config.h"

#ifdef BUILD_WITH_KIRANACCOUNTS
#include "kiran-accounts-manager.h"
#else
#include <act/act.h>
#endif

class MenuUserInfo : public sigc::trackable
{
public:
    MenuUserInfo(uid_t id);
    ~MenuUserInfo();

    bool is_ready() const;
    bool load();

    const char *get_username() const;
    const char *get_iconfile() const;

    sigc::signal<void> signal_ready();
    sigc::signal<void> signal_changed();

    static void on_loaded(MenuUserInfo *info);
    static void on_changed(MenuUserInfo *info);

private:
    sigc::signal<void> m_signal_ready;
    sigc::signal<void> m_signal_changed;

    uid_t uid;
#ifdef BUILD_WITH_KIRANACCOUNTS
    KiranAccountsUser *user;
#else
    ActUser *user;
#endif

    guint handler_id;
    int load_state;
};

#endif  // MENU_USER_INFO_H
