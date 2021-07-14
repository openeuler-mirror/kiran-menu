/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

#ifndef GLOBAL_INCLUDE_H
#define GLOBAL_INCLUDE_H

#include <gtkmm.h>

#if GTKMM_MINOR_VERSION < 24

namespace Gtk
{
template <class T, class... T_Args>
T* make_managed(T_Args&&... args)
{
    return manage(new T(std::forward<T_Args>(args)...));
}

};  // namespace Gtk
#endif

#endif
