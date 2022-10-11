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

#ifndef KIRAN_ACCOUNTS_MANAGER_H
#define KIRAN_ACCOUNTS_MANAGER_H
#include <glib-object.h>
#include <sys/types.h>
#include "kiran-accounts-user.h"

#define KIRAN_ACCOUNTS_BUS "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define KIRAN_ACCOUNTS_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts"

typedef struct _KiranAccountsManagerPrivate KiranAccountsManagerPrivate;
struct _KiranAccountsManager
{
    GObject parent;

    KiranAccountsManagerPrivate *priv;
};

#ifdef __cplusplus
extern "C"
{
#endif
#define KIRAN_ACCOUNTS_TYPE_MANAGER kiran_accounts_manager_get_type()

    G_DECLARE_FINAL_TYPE(KiranAccountsManager, kiran_accounts_manager, KIRAN_ACCOUNTS, MANAGER, GObject)

    KiranAccountsManager *kiran_accounts_manager_get_default();

    KiranAccountsUser *kiran_accounts_manager_get_user_by_id(KiranAccountsManager *self, uid_t uid);
#ifdef __cplusplus
}
#endif

#endif
