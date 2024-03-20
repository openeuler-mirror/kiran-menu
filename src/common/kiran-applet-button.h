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

#ifndef KIRANAPPLETBUTTON_H
#define KIRANAPPLETBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class KiranAppletButton : public Gtk::ToggleButton
{
public:
    KiranAppletButton(MatePanelApplet *applet_);
    ~KiranAppletButton() override;

    MatePanelApplet *get_applet();

protected:
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void set_icon_from_theme(const std::string &name);
    void generate_pixbuf();

private:
    MatePanelApplet *applet;

    std::string icon_name;
    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf;  //图标
    int icon_size;                          //图标显示尺寸
};

#endif  // KIRANAPPLETBUTTON_H
