/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:27:42
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-08 17:24:47
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/menu-new.h
 */

#pragma once

#include "lib/app.h"
#include "lib/menu-unit.h"

namespace Kiran
{
class MenuNew : public MenuUnit
{
   public:
    MenuNew();
    virtual ~MenuNew();

    virtual void init();

    virtual void flush(const AppVec &apps);

    std::vector<std::string> get_nnew_apps(gint top_n);

    void remove_from_new_apps(std::shared_ptr<App> app);

    //signal accessor:
    sigc::signal<void> &signal_new_app_changed() { return this->new_app_changed_; }

   private:
    void app_installed(AppVec apps);
    void app_uninstalled(AppVec apps);
    void app_launched(std::shared_ptr<App> app);

    void read_new_apps();
    void write_new_apps();

   protected:
    sigc::signal<void> new_app_changed_;

   private:
    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> new_apps_;
};

}  // namespace Kiran