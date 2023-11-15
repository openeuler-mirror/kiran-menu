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

#ifndef SHOWDESKTOPAPPLETBUTTON_H
#define SHOWDESKTOPAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class ShowDesktopAppletButton : public Gtk::ToggleButton
{
public:
    explicit ShowDesktopAppletButton(MatePanelApplet *applet_);

protected:
    virtual void on_map() override;
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_show_desktop_changed(bool show);
    virtual void on_toggled();

private:
    MatePanelApplet *applet;

    Gtk::Orientation get_applet_orientation() const;
};

#endif  // SHOWDESKTOPAPPLETBUTTON_H