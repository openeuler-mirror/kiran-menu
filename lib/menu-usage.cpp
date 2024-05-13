/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "lib/menu-usage.h"

#include <libwnck/libwnck.h>

#include "lib/app-manager.h"
#include "lib/base.h"
#include "lib/common.h"
#include "lib/math-helper.h"

namespace Kiran
{
#define FOCUS_TIME_MIN_SECONDS 2
#define FOCUS_TIME_MAX_SECONDS 1000
#define EPS 1e-8

/* If we transition to idle, only count this many seconds of usage */
#define IDLE_TIME_TRANSITION_SECONDS 30

/* http://www.gnome.org/~mccann/gnome-session/docs/gnome-session.html#org.gnome.SessionManager.Presence */
#define GNOME_SESSION_STATUS_IDLE 3

#define SAVE_APPS_TIMEOUT_SECONDS 10

MenuUsage::MenuUsage()
{
    this->settings_ = Gio::Settings::create(STARTMENU_SCHEMA);

    this->screen_idle_ = false;

    auto session_bus = Gio::DBus::Connection::get_sync(Gio::DBus::BusType::BUS_TYPE_SESSION);

    this->session_proxy_ = Gio::DBus::Proxy::create_sync(session_bus,
                                                         "org.gnome.SessionManager",
                                                         "/org/gnome/SessionManager/Presence",
                                                         "org.gnome.SessionManager");
}

MenuUsage::~MenuUsage()
{
    if (this->save_id_)
    {
        this->save_id_.disconnect();
    }
}

void MenuUsage::init()
{
    read_usages_from_settings();

    this->session_proxy_->signal_signal().connect(sigc::mem_fun(this, &MenuUsage::session_proxy_signal));
    WindowManager::get_instance()->signal_active_window_changed().connect(sigc::mem_fun(this, &MenuUsage::active_window_changed));
}

void MenuUsage::flush(const AppVec &apps)
{
}

std::vector<std::string> MenuUsage::get_nfrequent_apps(gint top_n)
{
    std::vector<std::string> apps;

    for (auto iter = this->app_usages_.begin(); iter != this->app_usages_.end(); ++iter)
    {
        if (iter->second.score > EPS)
        {
            Glib::QueryQuark query_quark((GQuark)iter->first);
            Glib::ustring desktop_id = query_quark;
            apps.push_back(desktop_id.raw());
        }
    }

    std::sort(apps.begin(), apps.end(), [this](const std::string &a, const std::string &b) -> bool
              {
        Glib::Quark quarka(a);
        Glib::Quark quarkb(b);

        auto itera = this->app_usages_.find(quarka.id());
        auto iterb = this->app_usages_.find(quarkb.id());

        double a_score = (itera == this->app_usages_.end()) ? 0 : itera->second.score;
        double b_score = (iterb == this->app_usages_.end()) ? 0 : iterb->second.score;

        return a_score > b_score; });

    if (top_n > 0 && top_n < (int)apps.size())
    {
        apps.resize(top_n);
    }

    return apps;
}

void MenuUsage::reset()
{
    this->app_usages_.clear();
    this->focus_desktop_id_.clear();

    write_usages_to_settings();
    this->app_changed_.emit();
}

void MenuUsage::on_session_status_changed(uint32_t status)
{
    bool idle;

    idle = (status >= GNOME_SESSION_STATUS_IDLE);
    if (this->screen_idle_ == idle)
        return;

    this->screen_idle_ = idle;
    if (idle)
    {
        if (this->focus_desktop_id_.length() > 0)
        {
            auto end_time = this->watch_start_time_ + IDLE_TIME_TRANSITION_SECONDS;
            increment_usage_for_app_at_time(this->focus_desktop_id_, end_time);
        }
    }
    else
    {
        this->watch_start_time_ = get_system_time();
    }
}

void MenuUsage::session_proxy_signal(const Glib::ustring &sender_name, const Glib::ustring &signal_name, const Glib::VariantContainerBase &parameters)
{
    if (signal_name == "StatusChanged")
    {
        auto tp = Glib::VariantBase::cast_dynamic<Glib::Variant<std::tuple<uint32_t>>>(parameters);
        auto status = tp.get_child<uint32_t>(0);
        on_session_status_changed(status);
    }
}

long MenuUsage::get_system_time(void)
{
    return g_get_real_time() / G_TIME_SPAN_SECOND;
}

MenuUsage::UsageData &MenuUsage::get_usage_for_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    return this->app_usages_[quark.id()];
}

bool MenuUsage::write_usages_to_settings()
{
    this->save_id_.disconnect();

    using UsagePair = std::pair<Glib::ustring, Glib::VariantBase>;

    std::vector<UsagePair> apps;

    for (auto iter = this->app_usages_.begin(); iter != this->app_usages_.end(); ++iter)
    {
        std::tuple<double, int32_t> usage_entry(iter->second.score, iter->second.last_seen);
        UsagePair entry;

        Glib::QueryQuark query_quark((GQuark)iter->first);
        Glib::ustring desktop_id = query_quark;
        auto usage = Glib::Variant<std::tuple<double, int32_t>>::create(usage_entry);
        entry.first = desktop_id;
        entry.second = Glib::Variant<Glib::Variant<std::tuple<double, int32_t>>>::create(usage);

        apps.push_back(entry);
    }

    return this->settings_->set_value(STARTMENU_KEY_FREQUENT_APPS, Glib::Variant<std::vector<UsagePair>>::create(apps));
}

bool MenuUsage::read_usages_from_settings()
{
    Glib::VariantBase base;
    this->settings_->get_value(STARTMENU_KEY_FREQUENT_APPS, base);

    try
    {
        auto container_base = Glib::VariantBase::cast_dynamic<Glib::VariantContainerBase>(base);

        gsize child_num = container_base.get_n_children();
        for (gsize i = 0; i < child_num; ++i)
        {
            Glib::VariantBase child_base;
            container_base.get_child(child_base, i);

            if (child_base.get_type().get_string() != "{sv}")
            {
                KLOG_WARNING("the element format for frequen-apps in gsettings must be {sv}. but now it's %s\n", child_base.get_type().get_string().c_str());
                return false;
            }

            auto entry = Glib::VariantBase::cast_dynamic<Glib::Variant<std::pair<std::string, Glib::VariantBase>>>(child_base);

            auto entry_pair = entry.get();
            auto desktop_id = entry_pair.first;

            auto usage_var = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::Variant<std::tuple<double, int32_t>>>>(entry_pair.second);
            auto usage = usage_var.get();

            double score = usage.get_child<double>(0);
            int32_t last_seen = usage.get_child<int32_t>(1);

            Glib::Quark quark(desktop_id);
            auto iter = this->app_usages_.emplace(quark.id(), UsageData{score, last_seen});
            if (!iter.second)
            {
                return false;
            }
        }
    }
    catch (std::bad_cast &bc)
    {
        KLOG_WARNING("failed to read frequent-apps: %s\n", bc.what());
        return false;
    }

    return true;
}

void MenuUsage::ensure_queued_save()
{
    if (this->save_id_)
        return;

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->save_id_ = timeout.connect_seconds(sigc::mem_fun(this, &MenuUsage::write_usages_to_settings), SAVE_APPS_TIMEOUT_SECONDS);
}

void MenuUsage::increment_usage_for_app_at_time(const std::string &desktop_id, int32_t time)
{
    auto &usage = get_usage_for_app(desktop_id);

    usage.last_seen = time;

    guint elapsed = time - this->watch_start_time_;

    if (elapsed >= FOCUS_TIME_MIN_SECONDS)
    {
        elapsed = (elapsed > FOCUS_TIME_MAX_SECONDS) ? FOCUS_TIME_MAX_SECONDS : elapsed;
        usage.score += math_log10(elapsed);
    }

    ensure_queued_save();

    this->app_changed_.emit();
}

void MenuUsage::active_window_changed(std::shared_ptr<Window> prev_active_window, std::shared_ptr<Window> cur_active_window)
{
    int32_t cur_system_time = get_system_time();

    if (this->focus_desktop_id_.length() > 0)
    {
        increment_usage_for_app_at_time(this->focus_desktop_id_, cur_system_time);
        this->focus_desktop_id_.clear();
    }

    auto app = AppManager::get_instance()->lookup_app_with_window(cur_active_window);

    if (app && app->should_show())
    {
        this->focus_desktop_id_ = app->get_desktop_id();
        auto &usage_data = get_usage_for_app(this->focus_desktop_id_);
        usage_data.last_seen = cur_system_time;
    }
    this->watch_start_time_ = cur_system_time;
}

}  // namespace Kiran