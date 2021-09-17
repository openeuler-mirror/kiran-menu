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

#ifndef KIRAN_POWER_INCLUDE_H
#define KIRAN_POWER_INCLUDE_H

#include <giomm.h>

#define LOGOUT_MODE_INTERACTIVE 0
#define LOGOUT_MODE_NOW 1

namespace KiranPower
{
bool suspend();
bool hibernate();
bool shutdown();
bool reboot();
bool logout(int mode);
bool switch_user();
bool lock_screen();

bool can_suspend();
bool can_hibernate();
bool can_switchuser();
bool can_shutdown();
bool can_reboot();

}  // namespace KiranPower

#endif  //KIRAN_POWER_INCLUDE_H
