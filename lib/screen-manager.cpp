/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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

#include "lib/screen-manager.h"

void on_show_desktop_changed(WnckScreen *screen,
                             gpointer user_data)
{
    Kiran::ScreenManager *manager = reinterpret_cast<Kiran::ScreenManager *>(user_data);

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
