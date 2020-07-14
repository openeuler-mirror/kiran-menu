/*
 * @Author       : tangjie02
 * @Date         : 2020-07-09 11:03:43
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-14 09:44:53
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/taskbar-skeleton.cpp
 */

#include "lib/taskbar-skeleton.h"

#include "lib/common.h"
namespace Kiran
{
TaskBarSkeleton::TaskBarSkeleton(AppManager *app_manager) : app_manager_(app_manager)
{
    this->settings_ = Gio::Settings::create(KIRAN_TASKBAR_SCHEMA);
    this->fixed_apps_ = read_as_to_list_quark(this->settings_, TASKBAR_KEY_FIXED_APPS);
}

TaskBarSkeleton::~TaskBarSkeleton()
{
}

TaskBarSkeleton *TaskBarSkeleton::instance_ = nullptr;
void TaskBarSkeleton::global_init(AppManager *app_manager)
{
    instance_ = new TaskBarSkeleton(app_manager);
    instance_->init();
}

void TaskBarSkeleton::init()
{
    this->desktop_app_changed();

    this->app_manager_->signal_desktop_app_changed().connect(sigc::mem_fun(this, &TaskBarSkeleton::desktop_app_changed));

    this->settings_->signal_changed(TASKBAR_KEY_FIXED_APPS).connect(sigc::mem_fun(this, &TaskBarSkeleton::app_changed));
}

bool TaskBarSkeleton::add_fixed_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());
    if (iter == this->fixed_apps_.end())
    {
        auto app = this->app_manager_->lookup_app(desktop_id);
        if (app)
        {
            this->fixed_apps_.push_back(quark.id());

            AppVec add_apps = {app};
            this->fixed_app_added_.emit(add_apps);
            return write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        }
        else
        {
            g_debug("<%s> not found desktop_id: %s.", __FUNCTION__, desktop_id.c_str());
        }
    }
    return false;
}

bool TaskBarSkeleton::del_fixed_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());

    if (iter != this->fixed_apps_.end())
    {
        auto app = this->app_manager_->lookup_app(desktop_id);
        if (app)
        {
            this->fixed_apps_.erase(iter);

            AppVec delete_apps = {app};
            this->fixed_app_deleted_.emit(delete_apps);
            return write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        }
        else
        {
            g_debug("<%s> not found desktop_id: %s.", __FUNCTION__, desktop_id.c_str());
        }
    }
    return false;
}

std::shared_ptr<App> TaskBarSkeleton::lookup_fixed_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());

    if (iter != this->fixed_apps_.end())
    {
        return app_manager_->lookup_app(desktop_id);
    }
    return nullptr;
}

AppVec TaskBarSkeleton::get_fixed_apps()
{
    AppVec apps;
    for (auto iter = this->fixed_apps_.begin(); iter != this->fixed_apps_.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)*iter);
        Glib::ustring desktop_id = query_quark;
        auto app = this->app_manager_->lookup_app(desktop_id.raw());
        if (app)
        {
            apps.push_back(app);
        }
    }
    return apps;
}

void TaskBarSkeleton::desktop_app_changed()
{
    auto apps = this->app_manager_->get_should_show_apps();

    std::set<int32_t> app_set;

    for (int i = 0; i < apps.size(); ++i)
    {
        auto &app = apps[i];
        auto &desktop_id = app->get_desktop_id();
        Glib::Quark quark(desktop_id);
        app_set.insert(quark.id());
    }

    AppVec delete_apps;

    auto iter = std::remove_if(this->fixed_apps_.begin(), this->fixed_apps_.end(), [this, &delete_apps, &app_set](int32_t elem) -> bool {
        if (app_set.find(elem) == app_set.end())
        {
            Glib::QueryQuark query_quark((GQuark)elem);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                delete_apps.push_back(app);
            }
            return true;
        }
        return false;
    });

    if (iter != this->fixed_apps_.end())
    {
        this->fixed_apps_.erase(iter, this->fixed_apps_.end());
        write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        this->fixed_app_deleted_.emit(delete_apps);
    }
}

void TaskBarSkeleton::app_changed(const Glib::ustring &key)
{
    auto new_fixed_apps = read_as_to_list_quark(this->settings_, TASKBAR_KEY_FIXED_APPS);

    AppVec add_apps;
    AppVec delete_apps;

    for (auto iter = this->fixed_apps_.begin(); iter != this->fixed_apps_.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(new_fixed_apps.begin(), new_fixed_apps.end(), value) == new_fixed_apps.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                delete_apps.push_back(app);
            }
        }
    }

    for (auto iter = new_fixed_apps.begin(); iter != new_fixed_apps.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), value) == this->fixed_apps_.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                delete_apps.push_back(app);
            }
        }
    }

    this->fixed_apps_ = new_fixed_apps;

    if (delete_apps.size() > 0)
    {
        this->fixed_app_deleted_.emit(delete_apps);
    }

    if (add_apps.size() > 0)
    {
        this->fixed_app_added_.emit(add_apps);
    }
}
}  // namespace Kiran