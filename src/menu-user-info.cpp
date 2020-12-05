#include "menu-user-info.h"
#include <iostream>

MenuUserInfo::MenuUserInfo(uid_t id) :
    uid(id),
    user(nullptr)
{
}

MenuUserInfo::~MenuUserInfo() 
{
    
}

bool MenuUserInfo::load()
{
    auto manager = act_user_manager_get_default();

    user = act_user_manager_get_user_by_id(manager, uid);
    if (!act_user_is_loaded(user))
        handler_id = g_signal_connect_swapped(user, "notify::is-loaded", G_CALLBACK(MenuUserInfo::on_loaded), this);
    else
        on_loaded(this);

    return true;
}

bool MenuUserInfo::is_ready() const
{
    return user != nullptr && act_user_is_loaded(user);
}

void MenuUserInfo::on_loaded(MenuUserInfo *info)
{
    g_signal_handler_disconnect(info->user, info->handler_id);
    info->signal_ready().emit();
}

const char *MenuUserInfo::get_username() const
{
    return act_user_get_user_name(user);
}

const char *MenuUserInfo::get_iconfile() const
{
    return act_user_get_icon_file(user);
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
    info->m_signal_changed.emit();
}
