/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:33
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-03 19:44:04
 * @Description  : 维护APP的一些基本信息
 * @FilePath     : /kiran-menu-2.0/lib/app.h
 */

#pragma once

#include <giomm.h>
#include <giomm/desktopappinfo.h>

#include <string>

namespace Kiran
{
class App;
using AppVec = std::vector<std::shared_ptr<App>>;

enum class AppKind
{
    FLATPAK,
    DESKTOP,
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

    /**
     * @description: 启动指定的app
     * @return: 启动是否成功，成功返回TRUE，失败返回FALSE
     * @author: songchuanfei
    */
    bool launch();

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
};

}  // namespace Kiran
