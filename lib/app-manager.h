/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 15:30:08
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/app-manager.h
 */
#pragma once

#include "lib/app.h"
#include "lib/menu-unit.h"
#include "lib/window-manager.h"
#include "lib/window.h"

namespace Kiran
{
class AppManager
{
   public:
    virtual ~AppManager();

    static AppManager *get_instance() { return instance_; };

    static void global_init(WindowManager *window_manager);

    static void global_deinit() { delete instance_; };

    void init();

    void load_apps();

    void add_application(WnckApplication *wnck_application);

    void remove_application(WnckApplication *wnck_application);

    AppVec get_apps();

    AppVec get_should_show_apps();

    AppVec get_running_apps();

    std::shared_ptr<App> lookup_app(const std::string &desktop_id);

    std::shared_ptr<App> lookup_app_with_window(std::shared_ptr<Window> window);

    std::shared_ptr<App> lookup_app_with_xid(uint64_t xid);

    std::vector<std::string> get_all_sorted_apps();

    //signal accessor:
    sigc::signal<void, AppVec> &signal_app_installed() { return this->app_installed_; }
    sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->app_uninstalled_; }
    sigc::signal<void, std::shared_ptr<App>> &signal_app_launched() { return this->app_launched_; };

   private:
    AppManager(WindowManager *window_manager);

   private:
    std::shared_ptr<App> get_app_from_sandboxed_app(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_window_wmclass(std::shared_ptr<Window> window);
    std::shared_ptr<App> lookup_app_with_wmclass(const std::string &wmclass);
    std::shared_ptr<App> lookup_app_with_desktop_wmclass(const std::string &wmclass);
    std::shared_ptr<App> lookup_app_with_heuristic_basename(const std::string &name);
    std::shared_ptr<App> get_app_from_env(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_desktop(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_window_group(std::shared_ptr<Window> window);

    void add_window(std::shared_ptr<Window> window);
    void remove_window(std::shared_ptr<Window> window);

    std::string get_exec_name(const std::string &exec_str);
    void app_launched(std::shared_ptr<App> app);

   protected:
    sigc::signal<void, AppVec> app_installed_;
    sigc::signal<void, AppVec> app_uninstalled_;
    sigc::signal<void, std::shared_ptr<App>> app_launched_;

   private:
    static AppManager *instance_;

    WindowManager *window_manager_;

    std::map<int32_t, std::shared_ptr<App>> apps_;
    std::map<std::string, std::shared_ptr<App>> wmclass_apps_;

    std::map<uint64_t, std::weak_ptr<App>> xid_to_app_;
};

}  // namespace Kiran