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

#ifndef KIRAN_ACCOUNTS_USER_INCLUDE_H
#define KIRAN_ACCOUNTS_USER_INCLUDE_H

#include <glib-object.h>
#include <kiran-system-daemon/accounts-i.h>

typedef enum AccountsPasswordMode KiranAccountsPasswordMode;
typedef enum AccountsAccountType KiranAccountsAccountType;

#define KIRAN_ACCOUNTS_BUS "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define KIRAN_ACCOUNTS_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_USER_INTERFACE "com.kylinsec.Kiran.SystemDaemon.Accounts.User"

#define KIRAN_ACCOUNTS_TYPE_USER kiran_accounts_user_get_type()

typedef struct _KiranAccountsUserPrivate KiranAccountsUserPrivate;
struct _KiranAccountsUser
{
    GObject parent;

    KiranAccountsUserPrivate *priv;
};

#ifdef __cplusplus
extern "C"
{
#endif
    G_DECLARE_FINAL_TYPE(KiranAccountsUser, kiran_accounts_user, KIRAN_ACCOUNTS, USER, GObject)

    KiranAccountsUser *kiran_accounts_user_new(const char *object_path);
    gboolean kiran_accounts_user_get_is_loaded(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_name(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_real_name(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_icon_file(KiranAccountsUser *user);

    guint64 kiran_accounts_user_get_uid(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_shell(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_language(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_home_directory(KiranAccountsUser *user);
    const char *kiran_accounts_user_get_email(KiranAccountsUser *user);
    gboolean kiran_accounts_user_is_locked(KiranAccountsUser *user);
    gboolean kiran_accounts_user_is_system_account(KiranAccountsUser *user);
    gboolean kiran_accounts_user_is_automatic_login(KiranAccountsUser *user);

    const char *kiran_accounts_user_get_password_hint(KiranAccountsUser *user);

    KiranAccountsPasswordMode kiran_accounts_user_get_password_mode(KiranAccountsUser *user);
    KiranAccountsAccountType kiran_accounts_user_get_account_type(KiranAccountsUser *user);
#ifdef __cplusplus
}
#endif

#endif  //KIRAN_ACCOUNTS_USER_INCLUDE_H
