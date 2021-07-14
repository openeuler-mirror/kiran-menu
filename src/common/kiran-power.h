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
