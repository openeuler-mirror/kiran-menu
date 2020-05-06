#pragma once
#include <gio/gio.h>

gdouble kiran_math_loge(gdouble a)
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

gdouble kiran_math_log10(gdouble a)
{
    return kiran_math_loge(a) / kiran_math_loge(10);
}