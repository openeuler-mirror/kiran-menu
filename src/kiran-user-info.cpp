#include "kiran-user-info.h"
#include <iostream>

//ActUserManager *KiranUserInfo::manager{NULL};

void on_user_loaded(KiranUserInfo *info)
{
    g_signal_handler_disconnect(info->user, info->handler_id);
    info->signal_data_ready().emit();
}

KiranUserInfo::KiranUserInfo(uid_t id): uid(id)
{
    gboolean loaded;

    manager = NULL;
    if (!manager)
        manager = act_user_manager_get_default();
    user = act_user_manager_get_user_by_id(manager, uid);
    g_object_get(G_OBJECT(user), "is-loaded", &loaded, NULL);
    if (!act_user_is_loaded(user))
        handler_id = g_signal_connect_swapped(user, "notify::is-loaded", G_CALLBACK(on_user_loaded), this);
    else
        on_user_loaded(this);
}

bool KiranUserInfo::is_ready()
{
    return act_user_is_loaded(user);
}

const char *KiranUserInfo::get_username()
{
    return act_user_get_user_name(user);
}

const char *KiranUserInfo::get_iconfile()
{
    return act_user_get_icon_file(user);
}

KiranUserInfo::ready_signal_t KiranUserInfo::signal_data_ready()
{
    return m_signal_data_ready;
}
