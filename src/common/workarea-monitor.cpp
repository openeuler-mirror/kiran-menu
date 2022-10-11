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

#include "workarea-monitor.h"
#include <X11/Xlib.h>

GdkFilterReturn event_filter(GdkXEvent *ev, GdkEvent *event, gpointer data)
{
    WorkareaMonitor *monitor = static_cast<WorkareaMonitor *>(data);
    XEvent *xevent = (XEvent *)ev;

    if (xevent->type != PropertyNotify)
        return GDK_FILTER_CONTINUE;

    char *prop_name = XGetAtomName(xevent->xany.display, xevent->xproperty.atom);

    if (!strcmp(prop_name, "_NET_WORKAREA"))
    {
        monitor->signal_size_changed().emit();
    }

    XFree(prop_name);
    return GDK_FILTER_CONTINUE;
}

WorkareaMonitor::WorkareaMonitor(Glib::RefPtr<Gdk::Screen> &screen_) : screen(screen_)
{
    auto root = screen->get_root_window();
    root->add_filter(event_filter, this);
}

WorkareaMonitor::~WorkareaMonitor()
{
    auto root = screen->get_root_window();

    signal_size_changed().clear();
    root->remove_filter(event_filter, this);
}

sigc::signal<void> WorkareaMonitor::signal_size_changed()
{
    return m_size_changed;
}
