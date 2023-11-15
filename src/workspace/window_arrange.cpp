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

#include "window_arrange.h"

/* 假设变量如下：
        w: 窗口数
        v: 划分的列数。建议不超过4列，列数越多，准确度越低
        h: 记录前i个窗口摆放完后列与列之间的最大差值(精度为p)。这个值越大，说明可记录的状态越多，准确度就越高
        p: 窗口高度的精度，窗口高度会重新计算为p的整数倍
   则算法时间复杂度为：v*w*h^(v-1)。*/

#define MAX_TIME_COMPLEXITY 1000000
#define DIV_CEIL(a, b) ((a + (b - 1)) / b)

std::vector<std::vector<int16_t>> window_arrangement(std::vector<int> windows, int32_t v, int32_t p)
{
    std::vector<std::vector<int16_t>> result(v, std::vector<int16_t>());
    if (v == 1)
    {
        for (int i = 0; i < (int)windows.size(); i++)
            result[0].push_back(i);

        return result;
    }
    int32_t w = windows.size();
    // 根据上面时间复杂度公式，保证计算量不要超过MAX_TIME_COMPLEXITY的情况下使h值最大化
    int32_t h = (int32_t)pow(MAX_TIME_COMPLEXITY * 1.0 / (v * w), 1.0 / (v - 1));
    // 根据窗口精度调整
    for (size_t i = 0; i < windows.size(); ++i)
    {
        windows[i] = std::max((int32_t)std::round(windows[i] / p), 1);
        windows[i] = std::min(windows[i], h - 1);
    }

    std::vector<int> sort_windows(windows);
    std::sort(sort_windows.begin(), sort_windows.end(), std::greater<int>());
    int32_t max_colum_height = 0;
    for (int32_t i = 0; i < DIV_CEIL(w, v) && i < (int32_t)sort_windows.size(); ++i)
    {
        max_colum_height += sort_windows[i];
    }
    // 差值不会超过可能的最大列高度
    if (h > max_colum_height + 1)
    {
        h = max_colum_height + 1;
    }

    // 计算h^0, h^1, h^2 ... h^v
    std::vector<int32_t> hbase(v, 0);
    hbase[0] = 1;
    for (int i = 1; i < v; ++i)
    {
        hbase[i] = hbase[i - 1] * h;
    }
    std::vector<std::vector<int16_t>> dvalues(hbase[v - 1], std::vector<int16_t>(v));
    for (int i = 0; i < hbase[v - 1]; ++i)
    {
        int state = i;
        for (int j = 0; j < v; ++j)
        {
            dvalues[i][j] = state % h;
            state /= h;
        }
    }

    std::vector<std::vector<int16_t>> dp(w, std::vector<int16_t>(hbase[v - 1], -1));
    // 第一个放置在第v列
    dp[0][windows[0] * hbase[v - 2]] = v - 1;
    for (int i = 1; i < w; ++i)
    {
        for (int j = 0; j < hbase[v - 1]; ++j)
        {
            if (dp[i - 1][j] == -1)
            {
                continue;
            }

            auto &dvalue = dvalues[j];
            int32_t new_state = 0;

            // 第i个窗口放在第0列的情况处理
            int32_t min_pos = 0;
            for (int k = 1; k < v - 1; ++k)
            {
                if (dvalue[k] < dvalue[min_pos])
                {
                    min_pos = k;
                }
            }

            if (windows[i] <= dvalue[min_pos])
            {
                new_state = j - windows[i] * (1 - hbase[v - 1]) / (1 - h);
                dp[i][new_state] = 0;
            }
            else
            {
                // 第0列和第min_pos+1列交换
                new_state = j - dvalue[min_pos] * (1 - hbase[v - 1]) / (1 - h) + (windows[i] - dvalue[min_pos]) * hbase[min_pos];
                dp[i][new_state] = min_pos + 1;
            }

            // 第i个窗口放在第k+1列的情况处理
            for (int k = 0; k < v - 1; ++k)
            {
                int32_t new_value = dvalue[k] + windows[i];
                if (new_value >= h)
                {
                    continue;
                }

                new_state = j + windows[i] * hbase[k];
                dp[i][new_state] = k + 1;
            }
        }
    }

    // 查找w个窗口的最佳摆放（最大和最小组的高度差值最小）
    int32_t optimal_state = -1;
    int16_t optimal_dvalue = h;
    for (int i = 0; i < hbase[v - 1]; ++i)
    {
        if (dp[w - 1][i] == -1)
        {
            continue;
        }
        auto &dvalue = dvalues[i];
        int16_t maxvalue = *std::max_element(dvalue.begin(), dvalue.end());
        if (optimal_dvalue > maxvalue)
        {
            optimal_dvalue = maxvalue;
            optimal_state = i;
        }
    }

    // 理论上不应该出现找不到的情况，如果出现返回空
    if (optimal_state == -1)
    {
        return result;
    }

    // 查找最佳摆放下每列的窗口编号
    int32_t cur_state = optimal_state;
    std::vector<int16_t> real_columns(v, 0);
    for (int i = 0; i < v; ++i)
    {
        real_columns[i] = i;
    }

    for (int i = w - 1; i >= 0; --i)
    {
        int32_t column = dp[i][cur_state];
        int32_t real_column = real_columns[column];
        result[real_column].push_back(i);

        auto &dvalue = dvalues[cur_state];
        if (column == 0)
        {
            cur_state += windows[i] * (1 - hbase[v - 1]) / (1 - h);
        }
        else
        {
            if (dvalue[column - 1] >= windows[i])
            {
                cur_state -= windows[i] * hbase[column - 1];
            }
            else
            {
                cur_state = cur_state + (windows[i] - dvalue[column - 1]) * (1 - hbase[v - 1]) / (1 - h) - dvalue[column - 1] * hbase[column - 1];
                std::swap(real_columns[column], real_columns[0]);
            }
        }
    }
    // 如果最后的cur_state不为0则说明代码有问题
    // printf("state: %d.\n", cur_state);

    return result;
}

std::vector<std::vector<int16_t>> arrange_data(std::vector<int> data, uint32_t rows)
{
    std::vector<std::vector<int16_t>> results;
    std::vector<std::pair<int16_t, int>> new_data;

    int64_t *row_sum = nullptr;
    uint32_t num_rows = 0, row_index;
    int16_t index = 0;

    /*
         * 由于最终返回的结果中记录的是数据的索引，所以需要保留原数据的索引信息
         */
    for (auto num : data)
    {
        new_data.push_back(std::make_pair(index, num));
        index++;
    }

    /* 把数据按照从大到小排序 */
    std::sort(new_data.begin(), new_data.end(),
              [](const std::pair<int16_t, int> &a, const std::pair<int16_t, int> &b) -> bool
              {
                  return a.second > b.second;
              });

    /* 构造基本数据结构并进行初始化 */
    row_sum = new int64_t[rows];
    for (uint32_t i = 0; i < rows; i++)
    {
        results.push_back(std::vector<int16_t>());
        row_sum[i] = 0;
    }

    /*
         * 先排满指定的行数，然后将剩余的逐个放到总和最小的行中
         */
    for (auto pair : new_data)
    {
        int num = pair.second;
        if (num_rows >= rows)
        {
            int64_t min_sum = INT64_MAX;
            uint32_t min_row = 0;
            for (uint32_t j = 0; j < rows; j++)
            {
                if (min_sum >= row_sum[j])
                {
                    min_sum = row_sum[j];
                    min_row = j;
                }
            }
            row_index = min_row;
        }
        else
            row_index = num_rows;

        results.at(row_index).push_back(pair.first);
        row_sum[row_index] += num;
        if (num_rows < rows)
            num_rows++;
    }

    delete[] row_sum;
    return results;
}
