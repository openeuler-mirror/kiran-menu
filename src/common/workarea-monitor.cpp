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
