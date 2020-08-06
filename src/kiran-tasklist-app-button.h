#ifndef KIRANTASKITEM_H
#define KIRANTASKITEM_H

#include <gtkmm.h>
#include <app.h>
#include <mate-panel-applet.h>
#include "kiran-tasklist-app-previewer.h"
#include "kiranhelper.h"
#include "kiran-app-context-menu.h"

class KiranTasklistAppButton: public Gtk::EventBox
{
public:
    KiranTasklistAppButton(const std::shared_ptr<Kiran::App> &app);
    ~KiranTasklistAppButton();

    void on_previewer_opened();
    void refresh();
    const std::shared_ptr<Kiran::App> get_app();

    bool get_context_menu_opened();

protected:
    virtual Gtk::SizeRequestMode get_request_mode_vfunc() const override;
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width)  const  override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height)  const override;
    virtual void get_preferred_width_for_height_vfunc(int width,
                                                      int& minimum_height,
                                                      int& natural_height) const override;
    virtual void get_preferred_height_for_width_vfunc(int width,
                                                     int& minimum_height,
                                                     int& natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_button_release_event(GdkEventButton *button_event) override;

    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;

    virtual Gtk::Orientation get_orientation() const;


private:
    std::weak_ptr<Kiran::App> app;              //关联的app对象
    Gtk::DrawingArea drawing_area;
    Glib::RefPtr<Gdk::Window> window;             //事件窗口
    Gtk::StyleProperty<int> indicator_size_property; //绘制指示器的尺寸

    KiranAppContextMenu *context_menu;                      //右键菜单

    int icon_size;                                //绘制应用图标的尺寸
    std::weak_ptr<Kiran::Window> last_raised;     //上次点击时最前的窗口

    bool menu_opened;
};

#endif // KIRANTASKITEM_H
