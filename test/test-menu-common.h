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

#pragma once
#include <gio/gio.h>

#include "lib/app.h"
#include "lib/menu-skeleton.h"

#define CHECK_PROXY_CALL_ERR(result, error)                       \
    {                                                             \
        if (!result)                                              \
        {                                                         \
            g_printerr("proxy call error: %s\n", error->message); \
            g_error_free(error);                                  \
            return;                                               \
        }                                                         \
    }

#define CHECK_PROXY_CALL_ERR_WITH_RET(result, error, ret)         \
    {                                                             \
        if (!result)                                              \
        {                                                         \
            g_printerr("proxy call error: %s\n", error->message); \
            g_error_free(error);                                  \
            return ret;                                           \
        }                                                         \
    }
