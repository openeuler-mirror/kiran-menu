/*
 * @Author       : tangjie02
 * @Date         : 2020-06-11 19:51:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-15 15:31:13
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/screen-manager.cpp
 */

#include "lib/screen-manager.h"

void on_show_desktop_changed (WnckScreen *screen,
               gpointer    user_data)
{
    Kiran::ScreenManager *manager = reinterpret_cast<Kiran::ScreenManager*>(user_data);

    manager->signal_show_desktop_changed().emit(wnck_screen_get_showing_desktop(screen));
}

namespace Kiran
{
ScreenManager::ScreenManager()
{
    wnck_set_client_type(WNCK_CLIENT_TYPE_PAGER);
    this->screen_ = wnck_screen_get_default();

    g_signal_connect(this->screen_, "showing-desktop-changed", G_CALLBACK(on_show_desktop_changed), this);
}

ScreenManager::~ScreenManager()
{
}

ScreenManager *ScreenManager::instance_ = nullptr;
void ScreenManager::global_init()
{
    instance_ = new ScreenManager();
    instance_->init();
}

void ScreenManager::init()
{
}

void ScreenManager::force_update()
{
    g_return_if_fail(this->screen_ != NULL);

    wnck_screen_force_update(this->screen_);

    this->force_update_.emit();
}

void ScreenManager::set_show_desktop(bool show)
{
    g_return_if_fail(this->screen_ != NULL);
    wnck_screen_toggle_showing_desktop(this->screen_, show);
}

bool ScreenManager::get_show_desktop()
{
    return wnck_screen_get_showing_desktop(this->screen_);
}

}  // namespace Kiran
