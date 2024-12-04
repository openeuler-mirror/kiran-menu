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
