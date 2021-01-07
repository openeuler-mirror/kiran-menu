#ifndef TASKLISTPAGINGBUTTON_H
#define TASKLISTPAGINGBUTTON_H

#include <gtkmm.h>
#include <mate-panel-applet.h>

class TasklistPagingButton : public Gtk::Button
{
public:
    TasklistPagingButton(MatePanelApplet *applet_);

    void set_icon_image(const Glib::ustring icon_resource, int icon_size);

protected:
    virtual void on_clicked() override;
    virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time) override;
    virtual void on_drag_leave(const Glib::RefPtr<Gdk::DragContext>& context, guint time) override;

private:
    MatePanelApplet *applet;
    bool drag_triggered;
};

#endif // TASKLISTPAGINGBUTTON_H
