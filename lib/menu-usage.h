/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include "lib/menu-unit.h"

typedef struct _WnckWindow WnckWindow;
typedef struct _WnckScreen WnckScreen;

namespace Kiran
{
class MenuUsage : public MenuUnit
{
    struct UsageData
    {
        UsageData(double s = 0, int32_t l = 0)
        {
            score = s;
            last_seen = l;
        }
        double score;
        int32_t last_seen;
    };

public:
    MenuUsage();
    virtual ~MenuUsage();

    virtual void init();

    virtual void flush(const AppVec &apps);

    std::vector<std::string> get_nfrequent_apps(gint top_n);

    void reset();

    //signal accessor:
    sigc::signal<void()> &signal_app_changed() { return this->app_changed_; }

private:
    void on_session_status_changed(uint32_t status);
    void session_proxy_signal(const Glib::ustring &sender_name, const Glib::ustring &signal_name, const Glib::VariantContainerBase &parameters);
    long get_system_time(void);
    MenuUsage::UsageData &get_usage_for_app(const std::string &desktop_id);
    bool write_usages_to_settings();
    bool read_usages_from_settings();
    void ensure_queued_save();
    void increment_usage_for_app_at_time(const std::string &desktop_id, int32_t time);
    void active_window_changed(std::shared_ptr<Window> prev_active_window, std::shared_ptr<Window> cur_active_window);

protected:
    sigc::signal<void()> app_changed_;

private:
    Glib::RefPtr<Gio::Settings> settings_;

    std::map<uint32_t, UsageData> app_usages_;

    int32_t watch_start_time_;
    std::string focus_desktop_id_;

    sigc::connection save_id_;

    Glib::RefPtr<Gio::DBus::Proxy> session_proxy_;
    bool screen_idle_;
};

}  // namespace Kiran
