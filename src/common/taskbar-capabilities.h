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

#ifndef TASKBAR_CAPABILITIES_H
#define TASKBAR_CAPABILITIES_H

/**
 * @brief 任务栏固定能力查询接口
 *
 * 用于Menu插件右键菜单通过 is_taskbar_pinning_available() 判断是否显示“固定到任务栏”选项。
 * 除 TasklistAppletWidget 外，其他模块不应实例化该类，否则会导致能力误判。
 */
class TaskbarCapabilities
{
public:
    /**
     * @brief 查询当前进程是否存在任务栏固定能力
     * @return 存在 KiranTasklistApplet 实例时返回 true，否则返回 false
     */
    static bool is_taskbar_pinning_available();

    /**
     * @brief 任务栏固定能力注册令牌
     *
     * 由 TasklistAppletWidget 以值成员方式持有：
     * - 构造时增加进程内任务栏实例计数；
     * - 析构时减少计数；
     * - 禁止拷贝和移动，避免重复计数或所有权不明确。
     */
    class PinningProvider
    {
    public:
        PinningProvider();
        ~PinningProvider();

        PinningProvider(const PinningProvider &) = delete;
        PinningProvider &operator=(const PinningProvider &) = delete;
        PinningProvider(PinningProvider &&) = delete;
        PinningProvider &operator=(PinningProvider &&) = delete;
    };

private:
    static int pinning_provider_count_;
};

#endif  // TASKBAR_CAPABILITIES_H
