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

#pragma once

#include <giomm.h>

#define LOGOUT_MODE_INTERACTIVE 0
#define LOGOUT_MODE_NOW 1

/* 这里会优先调用会话管理的退出接口，如果会话管理未提供再使用login1的接口，因为会话管理会进行抑制器的处理，对用户体验更加友好，
例如当用户在编辑一个未保存的文本文件时，如果调用会话管理的关机接口，则先会提示用户存在未完成的应用，是否需要强制关机，
而调用login1的接口则不会由该提示信息。
TODO: 这里面的API需要拆分
*/

class KiranPower
{
public:
    virtual ~KiranPower(){};

    static std::shared_ptr<KiranPower> get_default();

    // 可以使用的TV数
    uint32_t get_ntvs_total();
    // 图形已经使用的TV数
    uint32_t get_graphical_ntvs();

    bool suspend();
    bool hibernate();
    bool shutdown();
    bool reboot();
    bool logout(int mode);
    bool switch_user();
    bool lock_screen();

    bool can_suspend();
    bool can_hibernate();
    bool can_shutdown();
    bool can_reboot();
    bool can_logout();
    bool can_switch_user();
    bool can_lock_screen();

private:
    KiranPower();

private:
    static std::shared_ptr<KiranPower> instance_;
    Glib::RefPtr<Gio::Settings> settings_;
    Glib::RefPtr<Gio::DBus::Proxy> login1_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> session_manager_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> seat_manager_proxy_;
};
