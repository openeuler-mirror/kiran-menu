#include "kiranpower.h"
#include <iostream>

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"


bool KiranPower::suspend()
{
    try {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                  LOGIN_MANAGER_DBUS,
                                                  LOGIN_MANAGER_PATH,
                                                  LOGIN_MANAGER_INTERFACE);

        proxy->call_sync("Suspend", container, 300);
        return true;
    } catch (const Gio::DBus::Error &e) {
        std::cout<<"Failed to request suspend method: "<<e.what().data()<<std::endl;
        return false;
    }
}

bool KiranPower::hibernate()
{
    try {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                  LOGIN_MANAGER_DBUS,
                                                  LOGIN_MANAGER_PATH,
                                                  LOGIN_MANAGER_INTERFACE);

        proxy->call_sync("Hibernate", container, 300);
        return true;
    } catch (const Gio::DBus::Error &e) {
        std::cout<<"Failed to request hibernate method: "<<e.what().data()<<std::endl;
        return false;
    }
}
bool KiranPower::shutdown()
{
    try {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                  SESSION_MANAGER_DBUS,
                                                  SESSION_MANAGER_PATH,
                                                  SESSION_MANAGER_INTERFACE);

        proxy->call_sync("RequestShutdown");
        return true;
    } catch (const Gio::DBus::Error &e) {
        //throw std::string("Failed to connect to login manager: ") + e.what().data();
        std::cout<<"Failed to request shutdown method: "<<e.what().data()<<std::endl;
        return false;
    }
}
bool KiranPower::reboot()
{
    try {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                                     SESSION_MANAGER_DBUS,
                                                                                     SESSION_MANAGER_PATH,
                                                                                     SESSION_MANAGER_INTERFACE);

        proxy->call_sync("RequestReboot");
        return true;
    } catch (const Gio::DBus::Error &e) {
        std::cerr<<"Failed to request reboot method: "<<e.what().data()<<std::endl;
        return false;
    }
}

bool KiranPower::logout(int mode)
{
    try {
        Glib::Variant<uint> variant = Glib::Variant<uint>::create(mode);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                  SESSION_MANAGER_DBUS,
                                                  SESSION_MANAGER_PATH,
                                                  SESSION_MANAGER_INTERFACE);

        proxy->call_sync("Logout", container);
        return true;
    } catch (const Gio::DBus::Error &e) {
        std::cerr<<"Failed to connect to login manager: "<<e.what().data()<<std::endl;
        return false;
    }
}

bool KiranPower::can_suspend()
{
    try {
        Glib::VariantBase result;
        Glib::ustring data;
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                  LOGIN_MANAGER_DBUS,
                                                  LOGIN_MANAGER_PATH,
                                                  LOGIN_MANAGER_INTERFACE);

        result = proxy->call_sync("CanSuspend").get_child();
        data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    } catch (const Gio::DBus::Error &e) {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        std::cerr<<"Failed to query CanSuspend: "<<e.what().data()<<std::endl;
        return true;
    }
}

bool KiranPower::can_hibernate()
{
    try {
        Glib::VariantBase result;
        Glib::ustring data;
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                  LOGIN_MANAGER_DBUS,
                                                  LOGIN_MANAGER_PATH,
                                                  LOGIN_MANAGER_INTERFACE);

        result = proxy->call_sync("CanHibernate").get_child();
        data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    } catch (const Gio::DBus::Error &e) {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        std::cerr<<"Failed to query CanSuspend: "<<e.what().data()<<std::endl;
        return true;
    }
}

bool KiranPower::switch_user()
{
    try {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                  DISPLAY_MANAGER_DBUS,
                                                  DISPLAY_MANAGER_SEAT_PATH,
                                                  DISPLAY_MANAGER_INTERFACE);

        proxy->call_sync("SwitchToGreeter", Glib::VariantContainerBase(), 300);
        return true;
    } catch (const Gio::DBus::Error &e) {
        std::cout<<"Failed to request SwitchToGreeter method: "<<e.what().data()<<std::endl;
        return false;
    }
}
