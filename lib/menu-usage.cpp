/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 20:35:20
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 10:29:27
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-usage.cpp
 */
#include "lib/menu-usage.h"

#include <libwnck/libwnck.h>

#include "lib/helper.h"
#include "lib/math-helper.h"
#include "lib/menu-common.h"
#include "lib/menu-skeleton.h"
#include "lib/menu-system.h"

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

static void s_active_window_changed(WnckScreen *screen,
                                    WnckWindow *previously_active_window,
                                    gpointer user_data)
{
    MenuUsage *self = (MenuUsage *)user_data;
    self->active_window_changed(screen, previously_active_window);
}

MenuUsage::MenuUsage()
{
    this->settings_ = Gio::Settings::create(KIRAN_MENU_SCHEMA);

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

    WnckScreen *screen = wnck_screen_get_default();
    if (screen)
    {
        wnck_screen_force_update(screen);
        g_signal_connect(screen, "active-window-changed", G_CALLBACK(s_active_window_changed), this);
    }
    else
    {
        g_warning("the default screen is NULL. please run in GUI application.");
    }

    this->session_proxy_->signal_signal().connect(sigc::mem_fun(this, &MenuUsage::session_proxy_signal));
}

void MenuUsage::flush(const AppVec &apps)
{
}

void MenuUsage::active_window_changed(WnckScreen *screen, WnckWindow *previously_active_window)
{
    int32_t cur_system_time = get_system_time();

    if (this->focus_desktop_id_.length() > 0)
    {
        increment_usage_for_app_at_time(this->focus_desktop_id_, cur_system_time);
        this->focus_desktop_id_.clear();
    }

    auto unit = MenuSkeleton::get_instance()->get_unit(MenuUnitType::KIRAN_MENU_TYPE_SYSTEM);
    auto menu_system = std::dynamic_pointer_cast<MenuSystem>(unit);
    WnckWindow *active_window = wnck_screen_get_active_window(screen);
    auto app = menu_system->lookup_apps_with_window(active_window);

    if (app)
    {
        this->focus_desktop_id_ = app->get_desktop_id();
        auto &usage_data = get_usage_for_app(this->focus_desktop_id_);
        usage_data.last_seen = cur_system_time;
    }
    else
    {
        g_debug("not found matching app for changed window: %s\n",
                active_window ? wnck_window_get_name(active_window) : "null window name");
    }
    this->watch_start_time_ = cur_system_time;
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

    std::sort(apps.begin(), apps.end(), [this](const std::string &a, const std::string &b) -> bool {
        Glib::Quark quarka(a);
        Glib::Quark quarkb(b);

        auto itera = this->app_usages_.find(quarka.id());
        auto iterb = this->app_usages_.find(quarkb.id());

        double a_score = (itera == this->app_usages_.end()) ? 0 : itera->second.score;
        double b_score = (iterb == this->app_usages_.end()) ? 0 : iterb->second.score;

        return a_score > b_score;
    });

    if (top_n > 0 && top_n < apps.size())
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

    return this->settings_->set_value("frequent-apps", Glib::Variant<std::vector<UsagePair>>::create(apps));
}

bool MenuUsage::read_usages_from_settings()
{
    Glib::VariantBase base;
    this->settings_->get_value("frequent-apps", base);

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
                g_warning("the element format for frequen-apps in gsettings must be {sv}. but now it's %s\n", child_base.get_type().get_string().c_str());
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
        g_warning("failed to read frequent-apps: %s\n", bc.what());
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

}  // namespace Kiran