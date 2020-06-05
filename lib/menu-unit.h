/*
 * @Author       : tangjie02
 * @Date         : 2020-05-11 11:19:56
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-03 17:30:50
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/menu-unit.h
 */

#pragma once

#include <giomm.h>

#include "lib/app.h"

namespace Kiran
{
class MenuUnit
{
   public:
    virtual void flush(const AppVec &apps) = 0;

   private:
    gint type;
};

}  // namespace Kiran
