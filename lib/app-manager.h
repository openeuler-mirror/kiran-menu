/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-09 09:48:48
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

    void load_desktop_apps();

    // 获取所有App列表(每个App对应一个desktop文件)
    AppVec get_apps();

    // 获取可以在菜单中显示的App列表
    AppVec get_should_show_apps();

    // 获取正在运行的App列表
    AppVec get_running_apps();

    // 通过desktop_id获取App
    std::shared_ptr<App> lookup_app(const std::string &desktop_id);

    // 通过窗口对象获取App
    std::shared_ptr<App> lookup_app_with_window(std::shared_ptr<Window> window);

    // 通过WnckApplication的xid获取App
    std::shared_ptr<App> lookup_app_with_xid(uint64_t xid);

    // 获取所有App的desktop_id，并根据desktop文件的Name字段进行排序
    std::vector<std::string> get_all_sorted_apps();

    // App安装时的信号
    sigc::signal<void, AppVec> &signal_app_installed() { return this->app_installed_; }
    // App卸载时的信号
    sigc::signal<void, AppVec> &signal_app_uninstalled() { return this->app_uninstalled_; }
    // 应用程序状态发生变化
    sigc::signal<void, std::shared_ptr<App>, AppAction> &signal_app_action_changed() { return this->signal_app_action_changed_; }

private:
    AppManager(WindowManager *window_manager);

private:
    std::shared_ptr<App> get_app_from_sandboxed_app(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_gapplication_id(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_window_wmclass(std::shared_ptr<Window> window);
    std::shared_ptr<App> lookup_app_with_wmclass(const std::string &wmclass);
    std::shared_ptr<App> lookup_app_with_desktop_wmclass(const std::string &wmclass);
    std::shared_ptr<App> lookup_app_with_heuristic_basename(const std::string &name);
    std::shared_ptr<App> get_app_from_env(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_desktop(std::shared_ptr<Window> window);
    std::shared_ptr<App> get_app_from_window_group(std::shared_ptr<Window> window);

    void clear_desktop_apps();

    // 启动一个应用时的信号处理
    static void app_opened(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data);
    // 关闭一个应用时的信号处理
    static void app_closed(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data);

    // 处理窗口打开信号
    void window_opened(std::shared_ptr<Window> window);
    // 处理窗口关闭信号
    void window_closed(std::shared_ptr<Window> window);

    std::string get_exec_name(const std::string &exec_str);

    void app_launched(std::shared_ptr<App> app);
    void app_close_all_windows(std::shared_ptr<App> app);
    // void app_open_new_window(std::shared_ptr<App> app);

protected:
    sigc::signal<void, AppVec> app_installed_;
    sigc::signal<void, AppVec> app_uninstalled_;
    sigc::signal<void, std::shared_ptr<App>, AppAction> signal_app_action_changed_;

private:
    static AppManager *instance_;

    WindowManager *window_manager_;

    std::map<std::string, std::shared_ptr<App>> apps_;
    std::map<std::string, std::weak_ptr<App>> wmclass_apps_;

    std::map<uint64_t, std::weak_ptr<App>> wnck_apps_;
};

}  // namespace Kiran