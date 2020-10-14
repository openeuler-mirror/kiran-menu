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
    if (orient == MATE_PANEL_APPLET_ORIENT_UP || orient == MATE_PANEL_APPLET_ORIENT_DOWN)
        hints[0] = std::max(natural.width, 0);
    else
        hints[0] = std::max(natural.height, 0);

    hints[1] = 0;
    mate_panel_applet_set_size_hints(applet, hints, 2, 0);
    delete hints;
}

TasklistAppletWidget::TasklistAppletWidget(MatePanelApplet *applet):
    container(applet),
    vbox(Gtk::ORIENTATION_VERTICAL),
    prev_btn(nullptr),
    next_btn(nullptr)
{
    init_ui();
    g_signal_connect(applet, "size-allocate", G_CALLBACK(on_applet_size_allocate), this);
    g_signal_connect(applet, "change-orient", G_CALLBACK(on_applet_orient_change), this);
}

void TasklistAppletWidget::on_page_changed()
{
    prev_btn->set_sensitive(container.has_previous_page());
    next_btn->set_sensitive(container.has_next_page());

    if (prev_btn->get_sensitive() || next_btn->get_sensitive())
        vbox.show_all();
    else
        vbox.hide();
}

void TasklistAppletWidget::on_applet_orient_changed()
{
    hbox.set_orientation(container.get_orientation());
}

void TasklistAppletWidget::init_ui()
{
    prev_btn = create_button("/kiran-tasklist/icon/go-previous", _("Previous"));
    next_btn = create_button("/kiran-tasklist/icon/go-next", _("Next"));

    prev_btn->set_valign(Gtk::ALIGN_FILL);
    next_btn->set_valign(Gtk::ALIGN_FILL);

    vbox.set_spacing(2);
    vbox.set_margin_start(5);
    vbox.set_margin_end(5);
    vbox.set_margin_top(5);
    vbox.set_margin_bottom(5);
    vbox.set_valign(Gtk::ALIGN_FILL);
    vbox.set_halign(Gtk::ALIGN_CENTER);
    vbox.pack_start(*prev_btn, true, true);
    vbox.pack_end(*next_btn, true, true);


    hbox.pack_start(container, true, true);
    hbox.pack_end(vbox, false, false);
    add(hbox);

    hbox.property_orientation().signal_changed().connect(
                [this]() -> void {
                    if (hbox.get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
                        vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
                        vbox.set_size_request(16, -1);
                    }
                    else {
                        vbox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
                        vbox.set_size_request(-1, 16);
                    }

                    container.update_orientation();
                    container.queue_resize();
                });


    hbox.set_orientation(container.get_orientation());

    container.signal_page_changed().connect(sigc::mem_fun(*this, &TasklistAppletWidget::on_page_changed));
    container.signal_map().connect(sigc::mem_fun(*this, &TasklistAppletWidget::on_page_changed));

    prev_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_previous_page));
    next_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_next_page));

    prev_btn->get_style_context()->add_class("tasklist-arrow-button");
    next_btn->get_style_context()->add_class("tasklist-arrow-button");
}

/**
 * @brief 创建显示给定图片和提示的按钮，该按钮将随父控件一起销毁
 * @param icon_resource     图标资源路径
 * @param tooltip_text      提示信息文本
 * @return                  创建的按钮
 */
Gtk::Button *TasklistAppletWidget::create_button(std::string icon_resource, std::string tooltip_text)
{
    Gtk::Button *button = nullptr;

    auto image = Gtk::make_managed<Gtk::Image>();
    image->set_pixel_size(16);
    image->set_from_resource(icon_resource);

    button = Gtk::make_managed<Gtk::Button>();
    button->add(*image);
    button->set_tooltip_text(tooltip_text);
    button->set_size_request(16, 16);

    return button;
}
