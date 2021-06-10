/**
 * @file          /kiran-menu/lib/app.h
 * @brief         维护APP的一些基本信息
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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

enum AppKind
{
    // 未知类型，正常情况不应该出现
    UNKNOWN = 0x0,
    // desktop文件在XDG_DATA_DIRS环境变量下
    NORMAL = 0x1,
    // 用户固定到任务栏上的自定义应用, desktop文件在用户目录下
    USER_TASKBAR = 0x2,
    // 无法对应到desktop文件，这里会伪造一个不存在的desktop_id，格式为"fake_${fake_id_count_}"
    FAKE_DESKTOP = 0x4,
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
    App(){};
    App(const App &) = delete;
    // 通过desktop_id创建App
    App(const std::string &desktop_id, AppKind kind_ = AppKind::NORMAL);
    virtual ~App();

    static std::shared_ptr<App> create_fake();
    static std::shared_ptr<App> create_from_file(const std::string &path, AppKind kind_ = AppKind::USER_TASKBAR);
    static std::shared_ptr<App> create_from_desktop_id(const std::string &id, AppKind kind_ = AppKind::NORMAL);

    void update_from_desktop_file(bool force = false);

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
    // 获取应用类型
    AppKind get_kind() { return this->kind_; }

    // 获取desktop文件中的Categories字段值
    std::string get_categories();

    // 获取actions列表名，例如 "new-window"，"new-private-window"
    std::vector<std::string> get_actions();

    // 获取action的名字，改名字可用于用户交互显示，例如"Open a New Window/新建窗口", "Open a New Private Window/新建隐私浏览窗口"
    std::string get_action_name(const std::string &action);

    // 获取desktop文件中设置的图标
    const Glib::RefPtr<Gio::Icon> get_icon();

    std::string get_startup_wm_class();

    // 判断是否应该在菜单中显示
    bool should_show();

    // 是否为当前活动应用（当前活动窗口是否属于该应用)
    bool is_active();

    // 获取当前App对应的已打开的窗口列表
    WindowVec get_windows();

    // 获取任务栏中显示的窗口列表
    WindowVec get_taskbar_windows();

    // 关闭当前App对应的所有窗口
    void close_all_windows();

    // 启动应用，成功返回true，失败返回false
    bool launch();

    // 启动应用打开给定的uri，成功返回true,失败返回false
    bool launch_uris(const Glib::ListHandle<std::string> &uris);

    // 通过action_name启动应用，例如"new-window"，"new-private-window"
    void launch_action(const std::string &action_name);

    // 添加WnckApplication的xid
    void add_wnck_app_by_xid(uint64_t xid) { wnck_apps_.insert(xid); };
    // 删除WnckApplication的xid
    void del_wnck_app_by_xid(uint64_t xid) { wnck_apps_.erase(xid); };
    // 获取WnckApplication数量
    int32_t get_wnck_app_count() { return this->wnck_apps_.size(); }

protected:
    // 通过调用App::launch启动应用成功的信号
    sigc::signal<void, std::shared_ptr<App>> signal_launched() { return this->launched_; }
    // 通过调用App::launch启动应用失败的信号
    sigc::signal<void, std::shared_ptr<App>> signal_launch_failed() { return this->launch_failed_; }
    // 关闭所有窗口信号
    sigc::signal<void, std::shared_ptr<App>> signal_close_all_windows() { return this->close_all_windows_; }
    // 打开一个新窗口信号
    // sigc::signal<void, std::shared_ptr<App>> signal_open_new_window() { return this->open_new_window_; }

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

    bool x_kiran_no_display_;

    AppKind kind_;

    Glib::RefPtr<Gio::DesktopAppInfo> desktop_app_;

    std::set<uint64_t> wnck_apps_;

    static int32_t fake_id_count_;

    friend class AppManager;
};

}  // namespace Kiran
