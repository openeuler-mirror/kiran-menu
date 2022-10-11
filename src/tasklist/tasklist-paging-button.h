/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

#ifndef TASKLISTPAGINGBUTTON_H
#define TASKLISTPAGINGBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class TasklistPagingButton : public Gtk::Button
{
public:
    TasklistPagingButton(MatePanelApplet *applet_);

    void set_icon_image(const Glib::ustring icon_resource, int icon_size);

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
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
