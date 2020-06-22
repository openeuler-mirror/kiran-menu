/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-22 08:47:36
 * @Description  : 维护APP的一些基本信息
 * @FilePath     : /kiran-menu-2.0/lib/app.h
 */

#pragma once

#include <giomm.h>
#include <giomm/desktopappinfo.h>
#include <libwnck/libwnck.h>

#include <set>
#include <string>

#include "lib/window.h"

namespace Kiran
{
class App;
class Window;

using AppVec = std::vector<std::shared_ptr<Kiran::App>>;

enum class AppKind
{
    FLATPAK,
    DESKTOP,
    UNKNOWN,
};

enum class AppStatus
{
    RUNNING,
    STOP,
    UNKNOWN,
};

enum class AppAction : uint32_t
{
    // 通过调用App::launch启动应用成功的信号，如果需要监听所有启动的情况，建议使用APP_OPENED
    APP_LAUNCHED = (0 << 1),
    // 应用程序启动
    APP_OPENED = (1 << 1),
    // 应用程序关闭
    APP_CLOSED = (2 << 1),
    // 应用程序窗口列表变化
    APP_WINDOW_CHANGED = (3 << 1),
    // 应用程序所有窗口被关闭
    APP_ALL_WINDOWS_CLOSED = (4 << 1),
};

class App : public std::enable_shared_from_this<App>
{
public:
    App(const App &) = delete;
    App(const std::string &desktop_id);
    virtual ~App();

    // 获取desktop文件中的Name字段值
    const std::string &get_name() { return this->name_; }
    // 获取desktop文件中的Name[locale]字段值
    const std::string &get_locale_name() { return this->locale_name_; }
    // 获取desktop文件中的Comment字段值
    const std::string &get_comment() { return this->comment_; }
    // 获取desktop文件中的Comment[locale]字段值
    const std::string &get_locale_comment() { return this->locale_comment_; }
    // 获取desktop_id，即desktop文件名
    const std::string &get_desktop_id() { return this->desktop_id_; }
    // 获取desktop文件中的Exec字段值
    const std::string &get_exec() { return this->exec_; }
    // 获取desktop文件的文件全路径
    const std::string &get_file_name() { return this->file_name_; };

    // 获取desktop文件中的Categories字段值
    std::string get_categories();

    // 获取actions列表名，例如"Open a New Window", "Open a New Private Window"
    std::vector<std::string> get_actions();

    // 获取desktop文件中设置的图标
    const Glib::RefPtr<Gio::Icon> get_icon();

    std::string get_startup_wm_class();

    // 判断是否应该在菜单中显示
    bool should_show();

    // 获取当前App对应的已打开的窗口列表
    WindowVec get_windows();

    // 关闭当前App对应的所有窗口
    void close_all_windows();

    // 启动应用，成功返回true，失败返回false
    bool launch();

    // 通过action_name启动应用，例如"Open a New Window", "Open a New Private Window"
    void launch_action(const std::string &action_name);

    // 添加WnckApplication的xid，一个xid对应一个启动的应用，一个App可能启动多个应用
    void add_wnck_app_by_xid(uint64_t xid);
    // 删除xid
    void del_wnck_app_by_xid(uint64_t xid);

protected:
    // 通过调用App::launch启动应用成功的信号
    sigc::signal<void, std::shared_ptr<App>> signal_launched() { return this->launched_; }
    // 通过调用App::launch启动应用失败的信号
    sigc::signal<void, std::shared_ptr<App>> signal_launch_failed() { return this->launch_failed_; }
    // 关闭所有窗口信号
    sigc::signal<void, std::shared_ptr<App>> signal_close_all_windows() { return this->close_all_windows_; }
    // 打开一个新窗口信号
    // sigc::signal<void, std::shared_ptr<App>> signal_open_new_window() { return this->open_new_window_; }

private:
    void init_app_kind();

    void expand_macro(char macro, GString *exec);
    bool expand_application_parameters(int *argc, char ***argv, GError **error);
    bool launch_flatpak(GError **error);

protected:
    sigc::signal<void, std::shared_ptr<App>> launched_;
    sigc::signal<void, std::shared_ptr<App>> launch_failed_;
    sigc::signal<void, std::shared_ptr<App>> close_all_windows_;
    // sigc::signal<void, std::shared_ptr<App>> open_new_window_;

private:
    std::string desktop_id_;

    std::string file_name_;

    std::string name_;
    std::string locale_name_;

    std::string comment_;
    std::string locale_comment_;

    std::string exec_;

    std::string icon_name_;

    std::string path_;

    AppKind kind_;

    Glib::RefPtr<Gio::DesktopAppInfo> desktop_app_;

    std::set<uint64_t> xids_for_wnck_app_;

    std::set<uint64_t> windows_;

    friend class AppManager;
};

}  // namespace Kiran
