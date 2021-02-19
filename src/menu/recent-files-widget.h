#ifndef __RECENT_FILES_WIDGET_H__
#define __RECENT_FILES_WIDGET_H__

#include <gtkmm.h>
#include "recent-files-list-box.h"
#include "kiran-search-entry.h"

class RecentFilesWidget: public Gtk::Box
{
public:
    RecentFilesWidget();

protected:
    void init_ui();
    void on_recent_list_changed();
    void on_search_changed();

private:
    Gtk::Box box;
    KiranSearchEntry search_entry; 
    Gtk::ScrolledWindow scrolled;
    RecentFilesListBox list;
    Gtk::Label empty_label;
};

#endif // __RECENT_FILES_WIDGET_H__