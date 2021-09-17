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

#include "kiran-applet-button.h"
#include "kiran-helper.h"
#include "lib/base.h"

#define BUTTON_MARGIN 6

static void on_applet_size_change(MatePanelApplet *applet UNUSED,
                                  gint size UNUSED,
                                  gpointer userdata)
{
    KLOG_DEBUG("applet size changed");
    auto widget = reinterpret_cast<Gtk::Widget *>(userdata);
    widget->queue_resize();
}

KiranAppletButton::KiranAppletButton(MatePanelApplet *applet_)
{
    applet = applet_;
    g_object_ref(applet);
    g_signal_connect_after(applet, "change-size", G_CALLBACK(on_applet_size_change), this);

    get_style_context()->add_class("kiran-applet-button");
}

KiranAppletButton::~KiranAppletButton()
{
    g_object_unref(applet);
}

MatePanelApplet *KiranAppletButton::get_applet()
{
    return applet;
}

void KiranAppletButton::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    int size = mate_panel_applet_get_size(applet);
    MatePanelAppletOrient orient = mate_panel_applet_get_orient(applet);

    if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
        minimum_width = natural_width = size + 2 * BUTTON_MARGIN;
    else
        minimum_width = natural_width = size;
}

void KiranAppletButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    int size = mate_panel_applet_get_size(applet);
    MatePanelAppletOrient orient = mate_panel_applet_get_orient(applet);

    if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
        minimum_height = natural_height = size;
    else
        minimum_height = natural_height = size + 2 * BUTTON_MARGIN;
}

void KiranAppletButton::on_size_allocate(Gtk::Allocation &allocation)
{
    MatePanelAppletOrient orient;

    orient = mate_panel_applet_get_orient(applet);
    if (orient == MATE_PANEL_APPLET_ORIENT_UP ||
        orient == MATE_PANEL_APPLET_ORIENT_DOWN)
    {
        icon_size = allocation.get_height() - 2 * BUTTON_MARGIN;
    }
    else
        icon_size = allocation.get_width() - 2 * BUTTON_MARGIN;

    KLOG_DEBUG("icon size changed to %d", icon_size);
    Gtk::ToggleButton::on_size_allocate(allocation);
}

bool KiranAppletButton::on_draw(const ::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    auto context = get_style_context();
    int scale = get_scale_factor();

    allocation = get_allocation();
    if (icon_pixbuf && icon_pixbuf->get_width() != icon_size * scale)
    {
        /* 图标大小发生变化，需要重新生成pixbuf */
        icon_pixbuf.reset();
    }

    if (!icon_pixbuf)
        generate_pixbuf();

    if (!icon_pixbuf)
        return false;

    context->set_state(get_state_flags());
    context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());

    cr->scale(1.0 / scale, 1.0 / scale);
    //将图标绘制在按钮的正中心位置
    Gdk::Cairo::set_source_pixbuf(cr,
                                  icon_pixbuf,
                                  (allocation.get_width() * scale - icon_pixbuf->get_width()) / 2.0,
                                  (allocation.get_height() * scale - icon_pixbuf->get_height()) / 2.0);
    cr->paint();
    return false;
}

void KiranAppletButton::generate_pixbuf()
{
    int scale = get_scale_factor();

    KLOG_INFO("generate pixbuf for button, resource %s\n",
              icon_resource.c_str());
    icon_pixbuf = Gdk::Pixbuf::create_from_resource(icon_resource,
                                                    icon_size * scale,
                                                    icon_size * scale);
}

void KiranAppletButton::set_icon_from_resource(const std::string &resource)
{
    icon_resource = resource;

    icon_pixbuf.clear();
    if (get_realized())
    {
        queue_resize();
    }
}
