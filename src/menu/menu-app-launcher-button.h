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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#ifndef MENU_APPLICATION_LAUNCHER_BUTTON_H
#define MENU_APPLICATION_LAUNCHER_BUTTON_H

#include <gtkmm.h>

class MenuAppLauncherButton : public Gtk::Button
{
public:
    explicit MenuAppLauncherButton(const std::string &icon_name, const char *tooltip, const char *cmdline);

    sigc::signal<void> signal_app_launched();

protected:
    virtual void on_clicked() override;
    sigc::signal<void> m_signal_app_launched;

private:
    Gtk::Image icon;
    Glib::RefPtr<Gio::AppInfo> app;
};

#endif  // MENU_APPLICATION_LAUNCHER_BUTTON_H
