/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "tasklist-applet-widget.h"
#include <glibmm/i18n.h>
#include "global.h"
#include "tasklist-paging-button.h"

void on_applet_orient_change(MatePanelApplet *applet UNUSED,
                             gint orient UNUSED,
                             gpointer userdata)
{
    auto widget = reinterpret_cast<TasklistAppletWidget *>(userdata);
    widget->on_applet_orient_changed();
}

void on_applet_size_allocate(MatePanelApplet *applet UNUSED,
                             GdkRectangle *rect UNUSED,
                             gpointer userdata)
{
    int *hints;
    Gtk::Requisition min, natural;
    Gtk::Widget *widget = reinterpret_cast<Gtk::Widget *>(userdata);
    MatePanelAppletOrient orient;

    orient = mate_panel_applet_get_orient(applet);
    widget->get_preferred_size(min, natural);

    hints = new int[2];
    if (orient == MATE_PANEL_APPLET_ORIENT_UP || orient == MATE_PANEL_APPLET_ORIENT_DOWN)
    {
        hints[0] = std::max(natural.width, 0);
        hints[1] = min.width;
    }
    else
    {
        hints[0] = std::max(natural.height, 0);
        hints[1] = min.height;
    }

    mate_panel_applet_set_size_hints(applet, hints, 2, 0);
    delete[] hints;
}

TasklistAppletWidget::TasklistAppletWidget(MatePanelApplet *applet_) : button_box(Gtk::ORIENTATION_HORIZONTAL),
                                                                       prev_btn(nullptr),
                                                                       next_btn(nullptr),
                                                                       container(applet_),
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
    if (paging_check.connected())
        return;

    paging_check = Glib::signal_idle().connect(
        sigc::bind_return<bool>(
            sigc::mem_fun(*this, &TasklistAppletWidget::update_paging_buttons_state),
            false));
}

void TasklistAppletWidget::on_applet_orient_changed()
{
    switch (mate_panel_applet_get_orient(applet))
    {
    case MATE_PANEL_APPLET_ORIENT_DOWN:
    case MATE_PANEL_APPLET_ORIENT_UP:
        set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        break;
    }
}

void TasklistAppletWidget::init_ui()
{
    prev_btn = create_paging_button("kiran-tasklist-previous-symbolic", _("Previous"));
    next_btn = create_paging_button("kiran-tasklist-next-symbolic", _("Next"));

    button_box.set_spacing(2);
    button_box.set_margin_start(5);
    button_box.set_margin_end(5);
    button_box.pack_start(*prev_btn, true, true);
    button_box.pack_end(*next_btn, true, true);

    pack_start(container, true, true);
    pack_end(button_box, false, false);

    property_orientation().signal_changed().connect(
        [this]() -> void
        {
            if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
            {
                button_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
                button_box.set_size_request(16, -1);
            }
            else
            {
                button_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
                button_box.set_size_request(-1, 16);
            }

            container.update_orientation();
            container.queue_allocate();
        });

    on_applet_orient_changed();

    container.signal_page_changed().connect(sigc::mem_fun(*this, &TasklistAppletWidget::on_app_buttons_page_changed));

    prev_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_previous_page));
    next_btn->signal_clicked().connect(sigc::mem_fun(container, &TasklistButtonsContainer::move_to_next_page));
}

Gtk::Button *TasklistAppletWidget::create_paging_button(const std::string &icon_name, const std::string &tooltip_text)
{
    auto button = Gtk::make_managed<TasklistPagingButton>(applet);

    button->set_image_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
    button->set_tooltip_text(tooltip_text);

    return button;
}
