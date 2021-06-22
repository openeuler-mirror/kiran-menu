/**
 * @file          /kiran-menu/src/menu/kiran-accounts-user.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
