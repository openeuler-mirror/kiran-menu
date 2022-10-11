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

#include "lib/core_worker.h"

namespace Kiran
{

void init_backend_system()
{
    ScreenManager::global_init();

    WindowManager::global_init(ScreenManager::get_instance());

    AppManager::global_init(WindowManager::get_instance());

    WorkspaceManager::global_init(WindowManager::get_instance());

    MenuSkeleton::global_init(AppManager::get_instance());

    TaskBarSkeleton::global_init(AppManager::get_instance());

    ScreenManager::get_instance()->force_update();
}

void deinit_backend_system()
{
    TaskBarSkeleton::global_deinit();

    MenuSkeleton::global_deinit();

    WorkspaceManager::global_deinit();

    AppManager::global_deinit();

    WindowManager::global_deinit();

    ScreenManager::global_deinit();
}

}  // namespace Kiran
