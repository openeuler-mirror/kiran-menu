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
