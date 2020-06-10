/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 15:39:38
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

using AppVec = std::vector<std::shared_ptr<App>>;

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

class App : public std::enable_shared_from_this<App>
{
   public:
    App(const App &) = delete;
    App(const std::string &desktop_id);
    virtual ~App();

    const std::string &get_name() { return this->name_; }
    const std::string &get_locale_name() { return this->locale_name_; }
    const std::string &get_comment() { return this->comment_; }
    const std::string &get_locale_comment() { return this->locale_comment_; }
    const std::string &get_desktop_id() { return this->desktop_id_; }
    const std::string &get_exec() { return this->exec_; }
    const std::string &get_file_name() { return this->file_name_; };
    AppKind get_kind() { return this->kind_; };

    std::string get_categories();

    const Glib::RefPtr<Gio::Icon> get_icon();

    std::string get_startup_wm_class();

    bool should_show();

    WindowVec get_windows();

    void open_new_window();

    void close_all_windows();

    /**
     * @description: 启动指定的app
     * @return: 启动是否成功，成功返回TRUE，失败返回FALSE
     * @author: songchuanfei
    */
    bool launch();

    void add_wnck_app_by_xid(uint64_t xid);
    void del_wnck_app_by_xid(uint64_t xid);

    //signal accessor:
    sigc::signal<void, std::shared_ptr<App>> signal_launched() { return this->launched_; };
    sigc::signal<void, std::shared_ptr<App>> signal_launch_failed() { return this->launch_failed_; };

   private:
    void init_app_kind();

    void expand_macro(char macro, GString *exec);
    bool expand_application_parameters(int *argc, char ***argv, GError **error);
    bool launch_flatpak(GError **error);

   protected:
    sigc::signal<void, std::shared_ptr<App>> launched_;
    sigc::signal<void, std::shared_ptr<App>> launch_failed_;

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
};

}  // namespace Kiran
