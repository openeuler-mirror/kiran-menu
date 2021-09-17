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
