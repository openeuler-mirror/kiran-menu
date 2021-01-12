#ifndef WORKAREAMONITOR_H
#define WORKAREAMONITOR_H

#include <sigc++/sigc++.h>
#include <gtkmm.h>

class WorkareaMonitor : public sigc::trackable
{
public:
    WorkareaMonitor(Glib::RefPtr<Gdk::Screen> &screen);
    ~WorkareaMonitor();
    sigc::signal<void> signal_size_changed();


private:
    sigc::signal<void> m_size_changed;
    Glib::RefPtr<Gdk::Screen> screen;
};

#endif // WORKAREAMONITOR_H
