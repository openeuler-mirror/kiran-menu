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
