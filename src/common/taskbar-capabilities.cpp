/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-menu is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#include "taskbar-capabilities.h"

int TaskbarCapabilities::pinning_provider_count_ = 0;

TaskbarCapabilities::PinningProvider::PinningProvider()
{
    ++pinning_provider_count_;
}

TaskbarCapabilities::PinningProvider::~PinningProvider()
{
    if (pinning_provider_count_ > 0)
        --pinning_provider_count_;
}

bool TaskbarCapabilities::is_taskbar_pinning_available()
{
    return pinning_provider_count_ > 0;
}
