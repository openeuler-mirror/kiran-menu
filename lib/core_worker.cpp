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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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
