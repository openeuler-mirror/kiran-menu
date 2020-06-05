#include "lib/menu-favorite.h"

#include <set>

#include "lib/app.h"
#include "lib/menu-common.h"

namespace Kiran
{
MenuFavorite::MenuFavorite()
{
    this->settings = Gio::Settings::create(KIRAN_MENU_SCHEMA);

    this->favorite_apps = read_as_to_list_quark(this->settings, MENU_KEY_FAVORITE_APPS);

    this->settings->signal_changed(MENU_KEY_FAVORITE_APPS).connect(sigc::mem_fun(this, &MenuFavorite::app_changed));
}

MenuFavorite::~MenuFavorite()
{
}

void MenuFavorite::flush(const AppVec &apps)
{
    std::set<int32_t> app_set;

    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];
        auto &desktop_id = app->get_desktop_id();
        Glib::Quark quark(desktop_id);
        app_set.insert(quark.id());
    }

    std::vector<std::string> delete_apps;

    auto iter = std::remove_if(this->favorite_apps.begin(), this->favorite_apps.end(), [this, &delete_apps, &app_set](int32_t elem) -> bool {
        if (app_set.find(elem) == app_set.end())
        {
            Glib::QueryQuark query_quark((GQuark)elem);
            Glib::ustring desktop_id = query_quark;
            delete_apps.push_back(desktop_id.raw());
            return true;
        }
        return false;
    });

    if (iter != this->favorite_apps.end())
    {
        this->favorite_apps.erase(iter, this->favorite_apps.end());
        write_list_quark_to_as(this->settings, MENU_KEY_FAVORITE_APPS, this->favorite_apps);
        this->signal_app_deleted_.emit(delete_apps);
    }
}

gboolean MenuFavorite::add_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps.begin(), this->favorite_apps.end(), quark.id());
    if (iter == this->favorite_apps.end())
    {
        this->favorite_apps.push_back(quark.id());
        std::vector<std::string> add_apps = {desktop_id};
        this->signal_app_added_.emit(add_apps);
        return write_list_quark_to_as(this->settings, MENU_KEY_FAVORITE_APPS, this->favorite_apps);
    }
    return FALSE;
}

gboolean MenuFavorite::del_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps.begin(), this->favorite_apps.end(), quark.id());

    if (iter != this->favorite_apps.end())
    {
        this->favorite_apps.erase(iter);
        std::vector<std::string> delete_apps = {desktop_id};
        this->signal_app_deleted_.emit(delete_apps);
        return write_list_quark_to_as(this->settings, MENU_KEY_FAVORITE_APPS, this->favorite_apps);
    }
    return FALSE;
}

gboolean MenuFavorite::find_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->favorite_apps.begin(), this->favorite_apps.end(), quark.id());

    return (iter != this->favorite_apps.end());
}

std::vector<std::string> MenuFavorite::get_favorite_apps()
{
    std::vector<std::string> apps;
    for (auto iter = this->favorite_apps.begin(); iter != this->favorite_apps.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)*iter);
        Glib::ustring desktop_id = query_quark;
        apps.push_back(desktop_id.raw());
    }
    return apps;
}

void MenuFavorite::app_changed(const Glib::ustring &key)
{
    auto new_favorite_apps = read_as_to_list_quark(this->settings, MENU_KEY_FAVORITE_APPS);

    std::vector<std::string> add_apps;
    std::vector<std::string> delete_apps;

    for (auto iter = this->favorite_apps.begin(); iter != this->favorite_apps.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(new_favorite_apps.begin(), new_favorite_apps.end(), value) == new_favorite_apps.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            delete_apps.push_back(desktop_id);
        }
    }

    for (auto iter = new_favorite_apps.begin(); iter != new_favorite_apps.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(this->favorite_apps.begin(), this->favorite_apps.end(), value) == this->favorite_apps.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            add_apps.push_back(desktop_id);
        }
    }

    this->favorite_apps = new_favorite_apps;

    if (delete_apps.size() > 0)
    {
        signal_app_deleted_.emit(delete_apps);
    }

    if (add_apps.size() > 0)
    {
        signal_app_added_.emit(add_apps);
    }
}

}  // namespace Kiran