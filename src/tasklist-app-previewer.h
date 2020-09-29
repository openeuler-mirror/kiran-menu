#ifndef TASKLIST_APP_PREVIEWER_H
#define TASKLIST_APP_PREVIEWER_H

#include <gtkmm.h>
#include <X11/Xlib.h>
#include "tasklist-window-previewer.h"
#include "app.h"

#define DEBUG

class TasklistAppPreviewer : public Gtk::Window
{
public:
    TasklistAppPreviewer();

    void set_app(const std::shared_ptr<Kiran::App> &app);
    void set_idle(bool idle);

    const std::shared_ptr<Kiran::App> get_app() const;
    bool get_idle() const;

    void set_relative_to(Gtk::Widget *widget, Gtk::PositionType pos);
    void set_position(Gtk::PositionType pos, bool force = false);

    void reposition();
    void add_window_previewer(std::shared_ptr<Kiran::Window> &window, bool resize = false);
    void remove_window_previewer(std::shared_ptr<Kiran::Window> &window);
    uint32_t get_previewer_num();

    sigc::signal<void> signal_opened();

protected:
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual void adjust_size();
    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    //virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_show() override;

    virtual void on_child_remove();


    virtual void set_rgba_visual();


private:
    Gtk::ScrolledWindow scroll_window;
    Gtk::Box box;
    Glib::RefPtr<Gdk::Screen> screen;
    std::weak_ptr<Kiran::App> app;

    bool need_display;
    bool is_idle;

    Gtk::Widget *relative_to;
    Gtk::PositionType position;
    std::map<unsigned long, TasklistWindowPreviewer*> win_previewers;
    sigc::signal<void> m_signal_opened;

    void load_windows_list();
};

#endif // TASKLIST_APP_PREVIEWER_H
