#include "tasklist-applet-widget.h"
#include "global.h"
#include <glibmm/i18n.h>

void on_applet_orient_change(MatePanelApplet *applet UNUSED,
                             gint orient UNUSED,
                             gpointer userdata)
{
    auto widget = reinterpret_cast<TasklistAppletWidget*>(userdata);
    widget->on_applet_orient_changed();
}

void on_applet_size_allocate(MatePanelApplet *applet UNUSED,
                             GdkRectangle *rect UNUSED,
                             gpointer userdata)
{
    int *hints;
    Gtk::Requisition min, natural;
    Gtk::Widget *widget = reinterpret_cast<Gtk::Widget*>(userdata);
    MatePanelAppletOrient orient;

    orient = mate_panel_applet_get_orient(applet);
    widget->get_preferred_size(min, natural);

    hints = new int[2];
    if (orient == MATE_PANEL_APPLET_ORIENT_UP || orient == MATE_PANEL_APPLET_ORIENT_DOWN){
        hints[0] = std::max(natural.width, 0);
        hints[1] = min.width;
    }
    else {
        hints[0] = std::max(natural.height, 0);
        hints[1] = min.height;
    }

    mate_panel_applet_set_size_hints(applet, hints, 2, 0);
    delete hints;
}

TasklistAppletWidget::TasklistAppletWidget(MatePanelApplet *applet_):
    container(applet_),
    button_box(Gtk::ORIENTATION_VERTICAL),
    prev_btn(nullptr),
    next_btn(nullptr),
    applet(applet_)
{
    init_ui();
    g_signal_connect(applet, "size-allocate", G_CALLBACK(on_applet_size_allocate), this);
    g_signal_connect(applet, "change-orient", G_CALLBACK(on_applet_orient_change), this);
}

void TasklistAppletWidget::update_paging_buttons_state()
{
    prev_btn->set_sensitive(container.has_previous_page());
    next_btn->set_sensitive(container.has_next_page());

    button_box.set_visible(prev_btn->get_sensitive() || next_btn->get_sensitive());
}

void TasklistAppletWidget::on_app_buttons_page_changed()
{
    /*
     * 此处延缓更新分页按钮状态，否则不会触发size-allocate事件
     */
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &TasklistAppletWidget::update_paging_buttons_state));
}

void TasklistAppletWidget::on_applet_orient_changed()
{
    set_orientation(container.get_orientation());
}

void TasklistAppletWidget::init_ui()
{
    prev_btn = create_paging_button("/kiran-tasklist/icon/go-previous", _("Previous"));
    next_btn = create_paging_button("/kiran-tasklist/icon/go-next", _("Next"));

    prev_btn->set_valign(Gtk::ALIGN_FILL);
    next_btn->set_valign(Gtk::ALIGN_FILL);

    button_box.set_spacing(2);
    button_box.set_margin_start(5);
    button_box.set_margin_end(5);
    button_box.set_valign(Gtk::ALIGN_FILL);
    button_box.set_halign(Gtk::ALIGN_CENTER);
    button_box.pack_start(*prev_btn, true, true);
    button_box.pack_end(*next_btn, true, true);

    pack_start(container, true, true);
    pack_end(button_box, false, false);

    property_orientation().signal_changed().connect(
                [this]() -> void {
                    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
                        button_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
                        button_box.set_size_request(16, -1);
                    }
                    else {
                        button_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
                        button_box.set_size_request(-1, 16);
                    }

                    container.update_orientation();
                    container.queue_resize();
                });


    set_orientation(container.get_orientation());

    container.signal_page_changed().connect(sigc::mem_fun(*this, &TasklistAppletWidget::on_app_buttons_page_changed));

    prev_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_previous_page));
    next_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_next_page));
}

Gtk::Button *TasklistAppletWidget::create_paging_button(std::string icon_resource, std::string tooltip_text)
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::Button *button = nullptr;

    auto image = Gtk::make_managed<Gtk::Image>();
    image->set_pixel_size(16);
    image->set_from_resource(icon_resource);

    button = Gtk::make_managed<Gtk::Button>();
    button->add(*image);
    button->set_tooltip_text(tooltip_text);
    button->set_size_request(16, 16);
    button->get_style_context()->add_class("tasklist-arrow-button");

    button->signal_clicked().connect(
                [this, button]() -> void {
                    /*
                     * 获取输入焦点，当鼠标离开任务栏后当前窗口变化时，任务栏应用按钮会自动显示对应的状态
                     */
                    mate_panel_applet_request_focus(applet, gtk_get_current_event_time());

                    button->grab_focus();
                });


    /*
     * 当拖动的应用按钮经过分页按钮上方时，触发页面跳转
     */
    targets.push_back(Gtk::TargetEntry("text/plain"));
    button->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
    button->signal_drag_motion().connect(
                [button]( const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) -> bool {
                    button->clicked();
                    return true;
                });


    return button;
}
