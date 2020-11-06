#ifndef TASKLIST_APP_PREVIEWER_H
#define TASKLIST_APP_PREVIEWER_H

#include <gtkmm.h>
#include <X11/Xlib.h>
#include "tasklist-window-previewer.h"
#include "tasklist-app-button.h"
#include "app.h"

#define DEBUG

class TasklistAppPreviewer : public Gtk::Window
{
public:
    TasklistAppPreviewer();

    void set_app(const std::shared_ptr<Kiran::App> &app);

    const std::shared_ptr<Kiran::App> get_app() const;

    void set_relative_to(TasklistAppButton *button, Gtk::PositionType pos);
    void set_position(Gtk::PositionType pos);

    void reposition();
    void add_window_thumbnail(std::shared_ptr<Kiran::Window> &window);
    void remove_window_thumbnail(std::shared_ptr<Kiran::Window> &window);
    unsigned long get_thumbnails_count();

    bool has_context_menu_opened();

    void deferred_hide();
    void deferred_show();


    bool contains_pointer() const;

protected:
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void adjust_size();
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_scroll_event(GdkEventScroll *event) override;

    virtual void on_child_remove();

    virtual void on_child_context_menu_toggled(bool active);

    virtual void set_rgba_visual();
    virtual void init_ui();



private:
    Gtk::ScrolledWindow scroll_window;
    Gtk::Box box;
    Glib::RefPtr<Gdk::Screen> screen;
    std::weak_ptr<Kiran::App> app;

    TasklistAppButton *relative_to;
    Gtk::PositionType position;
    std::map<unsigned long, TasklistWindowPreviewer*> win_previewers;

    void load_windows_list();
    const Gtk::Scrollbar *get_scrollbar() const;
    Gtk::Scrollbar *get_scrollbar();

    static const int border_spacing;
};

#endif // TASKLIST_APP_PREVIEWER_H
