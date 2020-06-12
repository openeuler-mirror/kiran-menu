/*
 * @Author       : tangjie02
 * @Date         : 2020-06-11 19:51:25
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 19:54:16
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/screen-manager.h
 */

#pragma once
#include <libwnck/libwnck.h>

namespace Kiran
{
class ScreenManager
{
   public:
    virtual ~ScreenManager();

    static ScreenManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    void init();

   private:
    ScreenManager();

   private:
    static ScreenManager *instance_;

    WnckScreen *screen_;
};
}  // namespace Kiran
