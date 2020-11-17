#include "tasklist-paging-button.h"

TasklistPagingButton::TasklistPagingButton(MatePanelApplet *applet_):
    applet(applet_),
    drag_triggered(false)
{

    /*
     * 当拖动的应用按钮经过分页按钮上方时，触发页面跳转
     */
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("binary/app-id"));
    drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);

    get_style_context()->add_class("tasklist-arrow-button");
}

void TasklistPagingButton::set_icon_image(const Glib::ustring icon_resource, int icon_size)
{
    auto image = Gtk::make_managed<Gtk::Image>();
    image->set_pixel_size(icon_size);
    image->set_from_resource(icon_resource);

    add(*image);
}

void TasklistPagingButton::on_clicked()
{
    /*
     * 获取输入焦点，当鼠标离开任务栏后当前窗口变化时，任务栏应用按钮会自动显示对应的状态
     */
    mate_panel_applet_request_focus(applet, gtk_get_current_event_time());
    grab_focus();

    Gtk::Button::on_clicked();
}

bool TasklistPagingButton::on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    if (!drag_triggered) {
        clicked();
        drag_triggered = true;
    }
    context->drag_refuse(time);
    return true;
}

void TasklistPagingButton::on_drag_leave(const Glib::RefPtr<Gdk::DragContext> &context, guint time)
{
    drag_triggered = false;
}
