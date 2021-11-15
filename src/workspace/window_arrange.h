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

#ifndef WINDOW_ARRANGE_H
#define WINDOW_ARRANGE_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>
#include <cstdint>

std::vector<std::vector<int16_t>> window_arrangement(std::vector<int> windows, int32_t v = 4, int32_t p = 100);
std::vector<std::vector<int16_t>> arrange_data(std::vector<int> data, uint32_t rows);

#endif  // WINDOW_ARRANGE_H
