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

#ifndef TASKLISTPAGINGBUTTON_H
#define TASKLISTPAGINGBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class TasklistPagingButton : public Gtk::Button
{
public:
    explicit TasklistPagingButton(MatePanelApplet *applet_);

    // void set_icon_image(const Glib::ustring icon_resource, int icon_size);

protected:
    // virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_clicked() override;
    virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) override;
    virtual void on_drag_leave(const Glib::RefPtr<Gdk::DragContext> &context, guint time) override;

private:
    MatePanelApplet *applet;
    bool drag_triggered;
    Glib::ustring icon_resource;
    int icon_size;
};

#endif  // TASKLISTPAGINGBUTTON_H
