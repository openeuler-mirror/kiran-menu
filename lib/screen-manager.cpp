/*
 * @Author       : tangjie02
 * @Date         : 2020-06-11 19:51:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 19:54:36
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/screen-manager.cpp
 */

#include "lib/screen-manager.h"

namespace Kiran
{
ScreenManager::ScreenManager()
{
    this->screen_ = wnck_screen_get_default();
}

ScreenManager::~ScreenManager()
{
}

ScreenManager *ScreenManager::instance_ = nullptr;
void ScreenManager::global_init()
{
    instance_ = new ScreenManager();
    instance_->init();
}

void ScreenManager::init()
{
    if (this->screen_)
    {
        wnck_screen_force_update(this->screen_);
    }
}

}  // namespace Kiran