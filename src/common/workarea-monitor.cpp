#include "workarea-monitor.h"
#include <X11/Xlib.h>

GdkFilterReturn event_filter(GdkXEvent *ev, GdkEvent *event, gpointer data)
{
    WorkareaMonitor *monitor = static_cast<WorkareaMonitor*>(data);
    XEvent *xevent = (XEvent*)ev;

    if (xevent->type != PropertyNotify)
            return GDK_FILTER_CONTINUE;

    char *prop_name = XGetAtomName(xevent->xany.display, xevent->xproperty.atom);

    if (!strcmp(prop_name, "_NET_WORKAREA")) {
        monitor->signal_size_changed().emit();
    }

    XFree(prop_name);
    return GDK_FILTER_CONTINUE;
}

WorkareaMonitor::WorkareaMonitor(Glib::RefPtr<Gdk::Screen> &screen)
{
    auto root = screen->get_root_window();
    root->add_filter(event_filter, this);
}

sigc::signal<void> WorkareaMonitor::signal_size_changed()
{
    return m_size_changed;
}
