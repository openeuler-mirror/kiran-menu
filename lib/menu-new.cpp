/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:46
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-12 09:58:19
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/menu-new.cpp
 */

#include "menu-new.h"

#include <libwnck/libwnck.h>

#include <set>

#include "lib/app-manager.h"
#include "lib/common.h"
#include "lib/helper.h"
#include "lib/window-manager.h"

namespace Kiran
{
MenuNew::MenuNew()
{
    this->settings_ = Gio::Settings::create(KIRAN_MENU_SCHEMA);
}

MenuNew::~MenuNew()
{
}

void MenuNew::init()
{
    read_new_apps();

    AppManager::get_instance()->signal_app_installed().connect(sigc::mem_fun(this, &MenuNew::app_installed));
    AppManager::get_instance()->signal_app_uninstalled().connect(sigc::mem_fun(this, &MenuNew::app_uninstalled));
    AppManager::get_instance()->signal_app_action_changed().connect(sigc::mem_fun(this, &MenuNew::app_action_changed));
    WindowManager::get_instance()->signal_window_opened().connect(sigc::mem_fun(this, &MenuNew::window_opened));
}

void MenuNew::flush(const AppVec &apps)
{
    bool new_app_change = false;

    std::set<uint32_t> app_ids;
    for (auto iter = apps.begin(); iter != apps.end(); ++iter)
    {
        auto &app = *iter;
        Glib::Quark quark(app->get_desktop_id());
        app_ids.insert(quark.id());
    }

    auto iter = std::remove_if(this->new_apps_.begin(), this->new_apps_.end(), [&app_ids, &new_app_change](int32_t elem) -> bool {
        if (app_ids.find(elem) == app_ids.end())
        {
            new_app_change = true;
            return true;
        }
        return false;
    });

    if (iter != this->new_apps_.end())
    {
        this->new_apps_.erase(iter, this->new_apps_.end());
    }

    if (new_app_change)
    {
        write_new_apps();
    }
}

std::vector<std::string> MenuNew::get_nnew_apps(gint top_n)
{
    std::vector<std::string> new_apps;

    for (auto iter = this->new_apps_.begin(); iter != this->new_apps_.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)(*iter));
        Glib::ustring desktop_id = query_quark;
        new_apps.push_back(desktop_id);
    }

    if (top_n > 0 && top_n < new_apps.size())
    {
        new_apps.resize(top_n);
    }

    return new_apps;
}

void MenuNew::remove_from_new_apps(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);
    int32_t remove_value = quark.id();

    auto iter = std::remove_if(this->new_apps_.begin(), this->new_apps_.end(), [remove_value](int32_t elem) {
        return elem == remove_value;
    });

    if (iter != this->new_apps_.end())
    {
        this->new_apps_.erase(iter, this->new_apps_.end());
        write_new_apps();
    }
}

void MenuNew::app_installed(AppVec apps)
{
    bool new_app_change = false;
    for (auto iter = apps.begin(); iter != apps.end(); ++iter)
    {
        auto &app = (*iter);
        if (!app->should_show())
        {
            continue;
        }
        auto &desktop_id = app->get_desktop_id();
        Glib::Quark quark(desktop_id);
        if (std::find(this->new_apps_.begin(), this->new_apps_.end(), quark.id()) == this->new_apps_.end())
        {
            this->new_apps_.push_back(quark.id());
            new_app_change = true;
        }
    }
    if (new_app_change)
    {
        write_new_apps();
    }
}

void MenuNew::app_uninstalled(AppVec apps)
{
    bool new_app_change = false;

    std::set<uint32_t> app_ids;
    for (auto iter = apps.begin(); iter != apps.end(); ++iter)
    {
        auto &app = *iter;
        if (!app->should_show())
        {
            continue;
        }
        Glib::Quark quark(app->get_desktop_id());
        app_ids.insert(quark.id());
    }

    auto iter = std::remove_if(this->new_apps_.begin(), this->new_apps_.end(), [&app_ids, &new_app_change](int32_t elem) -> bool {
        if (app_ids.find(elem) != app_ids.end())
        {
            new_app_change = true;
            return true;
        }
        return false;
    });

    if (iter != this->new_apps_.end())
    {
        this->new_apps_.erase(iter, this->new_apps_.end());
    }

    if (new_app_change)
    {
        write_new_apps();
    }
}

void MenuNew::app_action_changed(std::shared_ptr<App> app, AppAction action)
{
    if (action == AppAction::APP_LAUNCHED)
    {
        remove_from_new_apps(app);
    }
}

void MenuNew::window_opened(std::shared_ptr<Window> window)
{
    auto app = AppManager::get_instance()->lookup_app_with_window(window);
    if (app)
    {
        this->remove_from_new_apps(app);
    }
}

void MenuNew::read_new_apps()
{
    this->new_apps_.clear();
    this->new_apps_ = read_as_to_list_quark(this->settings_, "new-apps");
}

void MenuNew::write_new_apps()
{
    write_list_quark_to_as(this->settings_, "new-apps", this->new_apps_);
    this->new_app_changed_.emit();
}

}  // namespace Kiran