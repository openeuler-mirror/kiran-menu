/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 15:37:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:24:58
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-favorite.h
 */
#pragma once

#include "lib/menu-unit.h"

namespace Kiran
{
class MenuFavorite : public MenuUnit
{
   public:
    MenuFavorite();
    virtual ~MenuFavorite();

    virtual void init();

    virtual void flush(const AppVec &apps);

    bool add_app(const std::string &desktop_id);

    bool del_app(const std::string &desktop_id);

    bool find_app(const std::string &desktop_id);

    std::vector<std::string> get_favorite_apps();

    //signal accessor:
    sigc::signal<void, std::vector<std::string>> &signal_app_added() { return this->app_added_; }
    sigc::signal<void, std::vector<std::string>> &signal_app_deleted() { return this->app_deleted_; }

   private:
    void app_changed(const Glib::ustring &key);

   protected:
    sigc::signal<void, std::vector<std::string>> app_added_;
    sigc::signal<void, std::vector<std::string>> app_deleted_;

   private:
    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> favorite_apps_;
};

}  // namespace Kiran