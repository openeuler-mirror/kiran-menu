#ifndef TASKLISTAPPLETWIDGET_H
#define TASKLISTAPPLETWIDGET_H

#include <gtkmm.h>
#include "tasklist-buttons-container.h"

class TasklistAppletWidget : public Gtk::EventBox
{
public:
    TasklistAppletWidget(MatePanelApplet *applet);
    void on_page_changed();
    void on_applet_orient_changed();

protected:
    void init_ui();
    Gtk::Button *create_button(std::string icon_resource, std::string tooltip_text);

private:
    Gtk::Button *prev_btn, *next_btn;
    TasklistButtonsContainer container;
    Gtk::Box hbox, vbox;
};

#endif // TASKLISTAPPLETWIDGET_H
