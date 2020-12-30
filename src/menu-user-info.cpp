#include "menu-user-info.h"
#include "log.h"
#include <iostream>

enum {
    INFO_STATE_NOT_LOAD,
    INFO_STATE_LOADING,
    INFO_STATE_LOADED
};

MenuUserInfo::MenuUserInfo(uid_t id) :
    uid(id),
    user(nullptr),
    load_state(INFO_STATE_NOT_LOAD)
{
}

MenuUserInfo::~MenuUserInfo() 
{
    
}

bool MenuUserInfo::load()
{
    if (load_state != INFO_STATE_NOT_LOAD)
	return true;

#ifdef BUILD_WITH_ACCOUNTSSERVICE
    auto manager = act_user_manager_get_default();

    user = act_user_manager_get_user_by_id(manager, uid);
    g_signal_connect_swapped(user, "changed", G_CALLBACK(&MenuUserInfo::on_changed), this);
    if (!act_user_is_loaded(user)) {
	load_state = INFO_STATE_LOADING;
        handler_id = g_signal_connect_swapped(user, "notify::is-loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    }
#else
    auto manager = kiran_accounts_manager_get_default();

    user = kiran_accounts_manager_get_user_by_id(manager, uid);
    g_signal_connect_swapped(user, "changed", G_CALLBACK(&MenuUserInfo::on_changed), this);
    if (!kiran_accounts_user_get_is_loaded(user)) {
	load_state = INFO_STATE_LOADING;
        handler_id = g_signal_connect_swapped(user, "loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    }
#endif
    else
        on_loaded(this);

    return true;
}

bool MenuUserInfo::is_ready() const
{
    return load_state == INFO_STATE_LOADED;
}

void MenuUserInfo::on_loaded(MenuUserInfo *info)
{
    info->load_state = INFO_STATE_LOADED;
    g_signal_handler_disconnect(info->user, info->handler_id);
    info->signal_ready().emit();
}

const char *MenuUserInfo::get_username() const
{
#ifdef BUILD_WITH_ACCOUNTSSERVICE
    return act_user_get_user_name(user);
#else
    return kiran_accounts_user_get_name(user);
#endif
}

const char *MenuUserInfo::get_iconfile() const
{
#ifdef BUILD_WITH_ACCOUNTSSERVICE
    return act_user_get_icon_file(user);
#else
    return kiran_accounts_user_get_icon_file(user);
#endif
}

sigc::signal<void> MenuUserInfo::signal_ready()
{
    return m_signal_ready;
}

sigc::signal<void> MenuUserInfo::signal_changed() 
{
    return m_signal_changed;
}

void MenuUserInfo::on_changed(MenuUserInfo *info)
{

    LOG_MESSAGE("got changed signal for user information");
    info->signal_changed().emit();
}
