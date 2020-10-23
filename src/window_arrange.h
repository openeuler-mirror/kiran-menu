#ifndef WINDOW_ARRANGE_H
#define WINDOW_ARRANGE_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

std::vector<std::vector<int16_t>> window_arrangement(std::vector<int> windows, int32_t v = 4, int32_t p = 100);
std::vector<std::vector<int16_t>> arrange_data(std::vector<int> data, uint32_t rows);


#endif // WINDOW_ARRANGE_H
