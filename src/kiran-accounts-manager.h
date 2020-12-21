#ifndef KIRAN_ACCOUNTS_MANAGER_H
#define KIRAN_ACCOUNTS_MANAGER_H
#include <sys/types.h>
#include <glib-object.h>
#include "kiran-accounts-user.h"

#define KIRAN_ACCOUNTS_BUS                  "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define KIRAN_ACCOUNTS_PATH                 "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define KIRAN_ACCOUNTS_INTERFACE            "com.kylinsec.Kiran.SystemDaemon.Accounts"

typedef struct _KiranAccountsManagerPrivate KiranAccountsManagerPrivate;
struct _KiranAccountsManager {
    GObject parent;

    KiranAccountsManagerPrivate *priv;
};

#ifdef __cplusplus
extern "C" {
#endif
#define KIRAN_ACCOUNTS_TYPE_MANAGER     kiran_accounts_manager_get_type()

G_DECLARE_FINAL_TYPE(KiranAccountsManager, kiran_accounts_manager, KIRAN_ACCOUNTS, MANAGER, GObject)

KiranAccountsManager *kiran_accounts_manager_get_default();

KiranAccountsUser *kiran_accounts_manager_get_user_by_id(KiranAccountsManager *self, uid_t uid);
#ifdef __cplusplus
}
#endif

#endif
