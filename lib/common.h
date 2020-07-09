/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 16:04:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-09 13:50:26
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/common.h
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
#define KIRAN_MENU_SCHEMA "com.unikylin.kiran.startmenu"

#define MENU_KEY_FAVORITE_APPS "favorite-apps"

#define MENU_KEY_FREQUENT_APPS "frequent-apps"

#define MENU_KEY_NEW_APPS "new-apps"

#define KIRAN_TASKBAR_SCHEMA "com.unikylin.kiran.taskbar"

#define TASKBAR_KEY_FIXED_APPS "fixed-apps"

#define RETURN_VAL_IF_INVALID_DESKTOP_ID(desktop_id, ret)                                        \
    {                                                                                            \
        auto app = app_manager_->lookup_app(desktop_id);                                         \
        if (!app)                                                                                \
        {                                                                                        \
            g_warning("<%s> not found the %s in AppManager.", __FUNCTION__, desktop_id.c_str()); \
            return ret;                                                                          \
        }                                                                                        \
        if (!(app->should_show()))                                                               \
        {                                                                                        \
            g_warning("<%s> the %s cannot show in menu.", __FUNCTION__, desktop_id.c_str());     \
            return ret;                                                                          \
        }                                                                                        \
    }

std::list<int32_t> read_as_to_list_quark(Glib::RefPtr<Gio::Settings> settings,
                                         const std::string &key);

bool write_list_quark_to_as(Glib::RefPtr<Gio::Settings> settings,
                            const std::string &key,
                            const std::list<int32_t> &value);

}  // namespace Kiran