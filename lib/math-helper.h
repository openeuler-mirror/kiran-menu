/*
 * @Author       : tangjie02
 * @Date         : 2020-06-02 08:53:16
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-02 08:53:44
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/math_helper.h
 */
#pragma once

#include <giomm.h>

namespace Kiran
{
gdouble math_loge(gdouble a)
{
    gint32 N = 10;
    gint32 k, nk;
    gdouble x, xx, y;
    x = (a - 1) / (a + 1);
    xx = x * x;
    nk = 2 * N + 1;
    y = 1.0 / nk;
    for (k = N; k > 0; k--)
    {
        nk = nk - 2;
        y = 1.0 / nk + xx * y;
    }
    return 2.0 * x * y;
}

gdouble math_log10(gdouble a)
{
    return math_loge(a) / math_loge(10);
}

}  // namespace Kiran