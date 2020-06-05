/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 15:37:28
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 20:22:53
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

    virtual void flush(const AppVec &apps);

    gboolean add_app(const std::string &desktop_id);

    gboolean del_app(const std::string &desktop_id);

    gboolean find_app(const std::string &desktop_id);

    std::vector<std::string> get_favorite_apps();

    //signal accessor:
    sigc::signal<void, std::vector<std::string>> &signal_app_added() { return this->signal_app_added_; }
    sigc::signal<void, std::vector<std::string>> &signal_app_deleted() { return this->signal_app_deleted_; }

   private:
    void app_changed(const Glib::ustring &key);

   protected:
    sigc::signal<void, std::vector<std::string>> signal_app_added_;
    sigc::signal<void, std::vector<std::string>> signal_app_deleted_;

   private:
    Glib::RefPtr<Gio::Settings> settings;

    std::list<int32_t> favorite_apps;
};

}  // namespace Kiran