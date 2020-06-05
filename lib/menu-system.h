/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 20:22:27
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-system.h
 */
#pragma once

#include "lib/app.h"
#include "lib/menu-unit.h"

typedef struct _WnckWindow WnckWindow;

namespace Kiran
{
class MenuSystem : public MenuUnit
{
   public:
    MenuSystem();
    virtual ~MenuSystem();

    virtual void flush(const AppVec &apps);

    AppVec get_apps();

    std::shared_ptr<App> lookup_app(const std::string &desktop_id);

    std::shared_ptr<App> lookup_apps_with_window(WnckWindow *window);

    std::vector<std::string> get_nnew_apps(gint top_n);

    std::vector<std::string> get_all_sorted_apps();

    void remove_from_new_apps(std::shared_ptr<App> app);

    //signal accessor:
    sigc::signal<void, AppVec> &signal_app_installed() { return this->signal_app_installed_; }
    sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->signal_app_uninstalled_; }
    sigc::signal<void> &signal_new_app_changed() { return this->signal_new_app_changed_; }

   private:
    gchar *get_exec_name(const gchar *exec_str);
    void read_new_apps();
    void write_new_apps();
    void app_launched(std::shared_ptr<App> app);

   protected:
    sigc::signal<void, AppVec> signal_app_installed_;
    sigc::signal<void, AppVec> signal_app_uninstalled_;
    sigc::signal<void> signal_new_app_changed_;

   private:
    Glib::RefPtr<Gio::Settings> settings;

    std::map<int32_t, std::shared_ptr<App> > apps;

    std::list<int32_t> new_apps;
};

}  // namespace Kiran