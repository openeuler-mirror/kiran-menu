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

#ifndef WINDOW_ARRANGE_H
#define WINDOW_ARRANGE_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

std::vector<std::vector<int16_t>> window_arrangement(std::vector<int> windows, int32_t v = 4, int32_t p = 100);
std::vector<std::vector<int16_t>> arrange_data(std::vector<int> data, uint32_t rows);

#endif  // WINDOW_ARRANGE_H
