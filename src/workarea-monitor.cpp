#include "workarea-monitor.h"
#include <X11/Xlib.h>
#include <gtk/gtkx.h>

GdkFilterReturn event_filter(GdkXEvent *ev, GdkEvent *event, gpointer data)
{
    WorkareaMonitor *monitor = static_cast<WorkareaMonitor*>(data);
    XEvent *xevent = (XEvent*)ev;
    GdkDisplay *display = nullptr;
    char *prop_name = nullptr;

    if (xevent->type != PropertyNotify)
        return GDK_FILTER_CONTINUE;

    display = gdk_window_get_display(gdk_event_get_window(event));

    gdk_x11_display_error_trap_push(display);
    prop_name = XGetAtomName(xevent->xany.display, xevent->xproperty.atom);
    if (gdk_x11_display_error_trap_pop(display))
        prop_name = nullptr;

    if (prop_name && !strcmp(prop_name, "_NET_WORKAREA")) {
        monitor->signal_size_changed().emit();
        XFree(prop_name);
    }

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
