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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "kiran-power.h"
#include <iostream>
#include "lib/base.h"

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_SESSION_INTERFACE "org.freedesktop.login1.Session"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"

#define DBUS_PROXY_TIMEOUT_MSEC 300

#define STARTMENU_LOCKDOWN_SCHEMA_ID "com.kylinsec.kiran.startmenu.lockdown"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOCK_SCREEN "disable-lock-screen"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_USER_SWITCHING "disable-user-switching"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_LOG_OUT "disable-log-out"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SUSPEND "disable-suspend"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_HIBERNATE "disable-hibernate"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_REBOOT "disable-reboot"
#define STARTMENU_LOCKDOWN_KEY_DISABLE_SHUTDOWN "disable-shutdown"

#define DEFAULT_N_AUTO_VTS 6

std::shared_ptr<KiranPower> KiranPower::instance_ = nullptr;
std::shared_ptr<KiranPower> KiranPower::get_default()
{
    if (!instance_)
    {
        instance_ = std::shared_ptr<KiranPower>(new KiranPower());
    }
    return instance_;
}

KiranPower::KiranPower()
{
    this->settings_ = Gio::Settings::create(STARTMENU_LOCKDOWN_SCHEMA_ID);

    try
    {
        this->login1_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                    LOGIN_MANAGER_DBUS,
                                                                    LOGIN_MANAGER_PATH,
                                                                    LOGIN_MANAGER_INTERFACE);

        this->session_manager_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                             SESSION_MANAGER_DBUS,
                                                                             SESSION_MANAGER_PATH,
                                                                             SESSION_MANAGER_INTERFACE);

        auto xdg_seat_object_path = Glib::getenv("XDG_SEAT_PATH");
        if (!xdg_seat_object_path.empty())
        {
            this->seat_manager_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                              DISPLAY_MANAGER_DBUS,
                                                                              xdg_seat_object_path,
                                                                              DISPLAY_MANAGER_INTERFACE);
        }
        else
        {
            KLOG_WARNING("Failed to get environment variable XDG_SEAT_PATH.");
        }
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }
}

uint32_t KiranPower::get_ntvs_total()
{
    try
    {
        Glib::VariantBase variant_value;
        this->login1_proxy_->get_cached_property(variant_value, "NAutoVTs");
        return Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(variant_value).get();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to request suspend method: %s", e.what().c_str());
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to request suspend method: %s", e.what());
    }
    return uint32_t(DEFAULT_N_AUTO_VTS);
}

uint32_t KiranPower::get_graphical_ntvs()
{
    try
    {
        Glib::VariantBase variant_value;
        this->seat_manager_proxy_->get_cached_property(variant_value, "Sessions");
        return Glib::VariantBase::cast_dynamic<Glib::VariantContainerBase>(variant_value).get_n_children();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }
    return 0;
}

bool KiranPower::suspend()
{
    RETURN_VAL_IF_FALSE(this->can_suspend(), false);

    try
    {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        this->login1_proxy_->call_sync("Suspend", container, DBUS_PROXY_TIMEOUT_MSEC);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request suspend method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::hibernate()
{
    RETURN_VAL_IF_FALSE(this->can_hibernate(), false);

    try
    {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        this->login1_proxy_->call_sync("Hibernate", container, 300);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request hibernate method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::shutdown()
{
    RETURN_VAL_IF_FALSE(this->can_shutdown(), false);

    try
    {
        this->session_manager_proxy_->call_sync("RequestShutdown");
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request shutdown method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::reboot()
{
    RETURN_VAL_IF_FALSE(this->can_reboot(), false);

    try
    {
        this->session_manager_proxy_->call_sync("RequestReboot");
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request reboot method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::logout(int mode)
{
    RETURN_VAL_IF_FALSE(this->can_logout(), false);

    try
    {
        Glib::Variant<uint> variant = Glib::Variant<uint>::create(mode);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        this->session_manager_proxy_->call_sync("Logout", container);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request logout method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::switch_user()
{
    RETURN_VAL_IF_FALSE(this->can_switch_user(), false);

    try
    {
        this->seat_manager_proxy_->call_sync("SwitchToGreeter", Glib::VariantContainerBase(), DBUS_PROXY_TIMEOUT_MSEC);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request SwitchToGreeter method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::lock_screen()
{
    RETURN_VAL_IF_FALSE(this->can_lock_screen(), false);

    std::vector<std::string> argv;

    if (access("/usr/bin/kiran-screensaver-command", F_OK) == 0)
    {
        argv.push_back("kiran-screensaver-command");
    }
    else
    {
        argv.push_back("mate-screensaver-command");
    }
    argv.push_back("-l");

    try
    {
        Glib::spawn_async("", argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_STDOUT_TO_DEV_NULL);
        return true;
    }
    catch (const Glib::SpawnError &e)
    {
        return false;
    }

    // FIXME: 由于kiran-applet是由dbus-daemon拉起，所以无法对应到会话ID，等后续使用自研kiran-side-panel后再使用下面的代码
    /*try
    {
        auto parameters = Glib::Variant<std::tuple<uint32_t>>::create(std::make_tuple<uint32_t>(getpid()));
        auto login1_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                  LOGIN_MANAGER_DBUS,
                                                                  LOGIN_MANAGER_PATH,
                                                                  LOGIN_MANAGER_INTERFACE);

        auto ret_variant = login1_proxy->call_sync("GetSessionByPID", parameters, DBUS_PROXY_TIMEOUT_MSEC);
        auto object_path = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::DBusObjectPathString>>(ret_variant.get_child(0)).get();

        auto login1_session_proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                          LOGIN_MANAGER_DBUS,
                                                                          object_path,
                                                                          LOGIN_SESSION_INTERFACE);

        login1_session_proxy->call_sync("Lock", Glib::VariantContainerBase(), DBUS_PROXY_TIMEOUT_MSEC);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to call LockScreen method: %s", e.what().c_str());
        return false;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to call LockScreen method: %s", e.what());
        return false;
    }
    return true;*/
}

bool KiranPower::can_suspend()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_SUSPEND), false);

    try
    {
        auto result = this->login1_proxy_->call_sync("CanSuspend").get_child();
        auto data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        KLOG_WARNING("Failed to query CanSuspend: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_hibernate()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_HIBERNATE), false);

    try
    {
        auto result = this->login1_proxy_->call_sync("CanHibernate").get_child();
        auto data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        KLOG_WARNING("Failed to query CanHibernate: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_shutdown()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_SHUTDOWN), false);

    try
    {
        auto result = this->login1_proxy_->call_sync("CanPowerOff").get_child();
        auto data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING("Failed to query CanPowerOff: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_reboot()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_REBOOT), false);

    try
    {
        auto result = this->login1_proxy_->call_sync("CanReboot").get_child();
        auto data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING("Failed to query Reboot: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_logout()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_LOG_OUT), false);
    return true;
}

bool KiranPower::can_switch_user()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_USER_SWITCHING), false);

    try
    {
        Glib::VariantBase variant_value;
        this->seat_manager_proxy_->get_cached_property(variant_value, "CanSwitch");
        return Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(variant_value).get();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }
    return true;
}

bool KiranPower::can_lock_screen()
{
    RETURN_VAL_IF_TRUE(this->settings_->get_boolean(STARTMENU_LOCKDOWN_KEY_DISABLE_LOCK_SCREEN), false);

    RETURN_VAL_IF_TRUE(access("/usr/bin/kiran-screensaver-command", F_OK) != 0 && access("/usr/bin/mate-screensaver-command", F_OK) != 0, false);

    return true;
}
