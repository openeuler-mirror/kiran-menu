/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 15:09:58
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/app.cpp
 */

#include "lib/app.h"

#include <cinttypes>
#include <vector>

#include "lib/helper.h"
#include "lib/log.h"
#include "lib/window-manager.h"

namespace Kiran
{
App::App(const std::string &desktop_id, AppKind kind) : kind_(kind),
                                                        desktop_id_(desktop_id)
{
}

App::~App()
{
}

int32_t App::fake_id_count_ = 0;
std::shared_ptr<App> App::create_fake()
{
    auto app = std::make_shared<App>();
    app->kind_ = AppKind::FAKE_DESKTOP;

    std::ostringstream oss;
    oss << "fake_" << ++App::fake_id_count_;
    app->desktop_id_ = oss.str();
    LOG_DEBUG("create fake app: %s.", app->desktop_id_.c_str());
    return app;
}

std::shared_ptr<App> App::create_from_file(const std::string &path, AppKind kind)
{
    auto desktop_app = Gio::DesktopAppInfo::create_from_filename(path);
    if (!desktop_app)
        return nullptr;
    auto app = std::make_shared<App>(desktop_app->get_id(), kind);

    app->desktop_app_ = desktop_app;
    app->update_from_desktop_file();
    return app;
}

std::shared_ptr<App> App::create_from_desktop_id(const std::string &id, AppKind kind)
{
    auto app = std::make_shared<App>(id, kind);
    app->desktop_app_ = Gio::DesktopAppInfo::create(id);
    app->update_from_desktop_file();
    return app;
}

void App::update_from_desktop_file(bool force)
{
    SETTINGS_PROFILE("id: %s.", this->desktop_id_.c_str());

    g_return_if_fail(this->desktop_app_);

    this->file_name_ = this->desktop_app_->get_filename();

    if (force)
        this->desktop_app_ = Gio::DesktopAppInfo::create_from_filename(this->file_name_);

#define GET_STRING(key) this->desktop_app_->get_string(key).raw()
#define GET_LOCALE_STRING(key) this->desktop_app_->get_locale_string(key).raw()

    this->name_ = GET_STRING("Name");
    this->locale_name_ = GET_LOCALE_STRING("Name");

    this->comment_ = GET_STRING("Comment");
    this->locale_comment_ = GET_LOCALE_STRING("Comment");

    this->exec_ = GET_STRING("Exec");

    this->icon_name_ = GET_STRING("Icon");

    this->path_ = GET_STRING("Path");

    this->x_kiran_no_display_ = this->desktop_app_->get_boolean("X-KIRAN-NoDisplay");

#undef GET_STRING
#undef GET_LOCALE_STRING
}

std::string App::get_categories()
{
    RETURN_VAL_IF_FALSE(this->desktop_app_, std::string());

    return this->desktop_app_->get_categories();
}

std::vector<std::string> App::get_actions()
{
    std::vector<std::string> raw_actions;

    RETURN_VAL_IF_FALSE(this->desktop_app_, raw_actions);

    auto actions = this->desktop_app_->list_actions();
    for (auto iter = actions.begin(); iter != actions.end(); ++iter)
    {
        raw_actions.push_back((*iter).raw());
    }
    return raw_actions;
}

std::string App::get_action_name(const std::string &action)
{
    RETURN_VAL_IF_FALSE(this->desktop_app_, std::string());
    return this->desktop_app_->get_action_name(action).raw();
}

const Glib::RefPtr<Gio::Icon> App::get_icon()
{
    RETURN_VAL_IF_FALSE(this->desktop_app_, Glib::RefPtr<Gio::Icon>());
    return this->desktop_app_->get_icon();
}

std::string App::get_startup_wm_class()
{
    RETURN_VAL_IF_FALSE(this->desktop_app_, std::string());
    return this->desktop_app_->get_startup_wm_class();
}

bool App::should_show()
{
    RETURN_VAL_IF_FALSE(this->desktop_app_, false);
    return (this->desktop_app_->should_show() && !this->x_kiran_no_display_);
}

bool App::is_active()
{
    auto window_manager = Kiran::WindowManager::get_instance();
    auto active_window = window_manager->get_active_window();

    if (active_window)
    {
        if (!active_window->should_skip_taskbar())
            return active_window->get_app().get() == this;
    }

    return false;
}

WindowVec App::get_windows()
{
    WindowVec windows;
    for (auto iter = this->wnck_apps_.begin(); iter != wnck_apps_.end(); ++iter)
    {
        auto xid = (*iter);
        auto wnck_app = wnck_application_get(xid);
        if (!wnck_app)
        {
            LOG_WARNING("cannot find the wnck_application. xid: %" PRIu64 "\n", xid);
            continue;
        }

        auto wnck_windows = wnck_application_get_windows(wnck_app);
        for (auto l = wnck_windows; l != NULL; l = l->next)
        {
            auto wnck_window = (WnckWindow *)(l->data);
            auto window = WindowManager::get_instance()->lookup_window(wnck_window);
            if (window)
            {
                windows.push_back(window);
            }
            else
            {
                LOG_WARNING("failed to lookup window, id: %" PRIu64 ", name: %s",
                            wnck_window_get_xid(wnck_window),
                            wnck_window_get_name(wnck_window));
            }
        }
    }
    return windows;
}

WindowVec App::get_taskbar_windows()
{
    auto windows = get_windows();
    auto iter = std::remove_if(windows.begin(), windows.end(), [](std::shared_ptr<Kiran::Window> window) {
        return window->should_skip_taskbar();
    });
    windows.erase(iter, windows.end());
    return windows;
}

void App::close_all_windows()
{
    SETTINGS_PROFILE("");

    for (auto iter = this->wnck_apps_.begin(); iter != wnck_apps_.end(); ++iter)
    {
        auto xid = (*iter);
        auto wnck_app = wnck_application_get(xid);
        if (!wnck_app)
        {
            LOG_WARNING("cannot find the wnck_application. xid: %" PRIu64 "\n", xid);
            continue;
        }

        LOG_DEBUG("close these windows belong to wnck_app<%" PRIu64 ", %s>.",
                  wnck_application_get_xid(wnck_app),
                  wnck_application_get_name(wnck_app));

        auto wnck_windows = wnck_application_get_windows(wnck_app);
        for (auto l = wnck_windows; l != NULL; l = l->next)
        {
            auto wnck_window = (WnckWindow *)(l->data);
            auto window = WindowManager::get_instance()->lookup_window(wnck_window);
            if (window)
            {
                window->close();
            }
            else
            {
                LOG_WARNING("failed to lookup window, id: %" PRIu64 ", name: %s",
                            wnck_window_get_xid(wnck_window),
                            wnck_window_get_name(wnck_window));
            }
        }
    }
    this->close_all_windows_.emit(this->shared_from_this());
}

bool App::launch()
{
    SETTINGS_PROFILE("id: %s.", this->desktop_id_.c_str());

    g_return_val_if_fail(this->desktop_app_, false);

    bool res = false;
    std::string error;
    auto app_context = Gdk::Display::get_default()->get_app_launch_context();

    try
    {
        std::vector<Glib::RefPtr<Gio::File> > files;
        res = this->desktop_app_->launch(files, app_context);
    }
    catch (const Glib::Error &e)
    {
        res = false;
        error = e.what().raw();
    }

    if (res)
    {
        this->launched_.emit(this->shared_from_this());
    }
    else
    {
        this->launch_failed_.emit(this->shared_from_this());
        LOG_WARNING("failed to launch: %s", error.c_str());
    }
    return res;
}

bool App::launch_uris(const Glib::ListHandle<std::string> &uris)
{
    SETTINGS_PROFILE("id: %s.", this->desktop_id_.c_str());

    g_return_val_if_fail(this->desktop_app_, false);
    g_warn_if_fail(uris.size() > 0);

    bool res = false;
    std::string error;
    auto app_context = Gdk::Display::get_default()->get_app_launch_context();

    try
    {
        std::vector<Glib::RefPtr<Gio::File> > files;
        res = this->desktop_app_->launch_uris(uris, app_context);
    }
    catch (const Glib::Error &e)
    {
        res = false;
        error = e.what().raw();
    }

    if (res)
    {
        this->launched_.emit(this->shared_from_this());
    }
    else
    {
        this->launch_failed_.emit(this->shared_from_this());
        LOG_WARNING("failed to launch uris: %s", error.c_str());
    }
    return res;
}

void App::launch_action(const std::string &action_name)
{
    SETTINGS_PROFILE("id: %s.", this->desktop_id_.c_str());

    g_return_if_fail(this->desktop_app_);

    auto app_context = Gdk::Display::get_default()->get_app_launch_context();
    this->desktop_app_->launch_action(action_name, app_context);
    // there is no way to detect failures that occur while using this function
    // this->launched_.emit(this->shared_from_this());
}

}  // namespace Kiran
