#include "kiran-tasklist-window-previewer.h"
#include <gtk/gtkx.h>
#include <cairomm/xlib_surface.h>
#include <iostream>
#include <glib/gi18n.h>

KiranWindowPreviewer::KiranWindowPreviewer(std::shared_ptr<Kiran::Window> &window_):
    KiranWindowThumbnail(window_),
    context_menu(nullptr)
{
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_no_show_all(true);
    set_spacing(10);

    //如果窗口管理器混合模式关闭，我们需要隐藏窗口预览图，因为窗口预览图已经无法获取
    signal_composited_changed().connect(sigc::mem_fun(*this, &KiranWindowPreviewer::on_composite_changed));

    get_style_context()->add_class("window-previewer");
    on_composite_changed();
}

KiranWindowPreviewer::~KiranWindowPreviewer()
{
    if (context_menu)
        delete context_menu;

}

bool KiranWindowPreviewer::draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{

    Gtk::Allocation allocation;
    double scale_x, scale_y, scale;
    Window xid;
    auto window = get_window_();

    if (!window)
        return true;

    allocation = snapshot_area->get_allocation();
    xid = window->get_xid();

    try {
        Drawable drawable = window->get_pixmap();
        XWindowAttributes attrs;
        Display *xdisplay = GDK_DISPLAY_XDISPLAY(get_display()->gobj());

        XGetWindowAttributes(xdisplay, xid, &attrs);
        if (drawable == None) {
            g_warning("Failed to get pixmap for window 0x%x\n", xid);
            if (attrs.map_state == IsViewable) {
                //无法从XComposite扩展获取到窗口截图，调用Xlib接口获取预览（仅针对非最小化的窗口)
                drawable = xid;
            } else {
                //如果无法获取到窗口截图，同时窗口不可见，那么在预览区域绘制窗口图标
                int pixbuf_width, pixbuf_height;
                GdkPixbuf* c_pixbuf = window->get_icon();
                if (!c_pixbuf)
                    return false;

                auto pixbuf = Glib::wrap(c_pixbuf, true);
                auto scaled = pixbuf->scale_simple(64, 64, Gdk::INTERP_BILINEAR);
                pixbuf_width = scaled->get_width();
                pixbuf_height = scaled->get_height();

                Gdk::Cairo::set_source_pixbuf(cr, scaled,
                                              (allocation.get_width() - pixbuf_width)/2,
                                              (allocation.get_height() - pixbuf_height)/2);
                cr->paint();
                return false;
            }
        }


        auto surface = Cairo::XlibSurface::create(xdisplay,
                                                  drawable,
                                                  attrs.visual,
                                                  attrs.width,
                                                  attrs.height);

        if (!surface) {
            std::cerr<<"Failed to get pixbuf for window"<<std::endl;
            return false;
        }

        scale_x = allocation.get_width() * 1.0/attrs.width;
        scale_y = allocation.get_height()* 1.0/attrs.height;

        scale = scale_x > scale_y?scale_y:scale_x;

        cr->translate((allocation.get_width() - attrs.width * scale)/2.0,
                      (allocation.get_height() - attrs.height * scale)/2.0);

        cr->scale(scale, scale);
        cr->set_source(surface, 0, 0);
        cr->paint();
    } catch (const Glib::Error &e) {
        std::cerr<<"Error occured while trying to draw window snapshot: "<<e.what()<<std::endl;
    }

    return false;
}

sigc::signal<void> KiranWindowPreviewer::signal_close()
{
    return m_signal_close;
}

void KiranWindowPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    GtkStateFlags flags;
    GValue value = G_VALUE_INIT;
    GtkWidget *widget = reinterpret_cast<GtkWidget*>(const_cast<GtkButton*>(this->gobj()));
    auto context = get_style_context()->gobj();

    flags = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_property(context, "min-width", flags, &value);
    minimum_width = g_value_get_int(&value);
    g_value_unset(&value);

    natural_width = minimum_width;
}

bool KiranWindowPreviewer::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    return KiranWindowThumbnail::on_draw(cr);
}

void KiranWindowPreviewer::on_thumbnail_clicked()
{
    auto window = get_window_();
    GdkEvent *event = gtk_get_current_event();

    if (gdk_event_triggers_context_menu(event)) {
        //show context menu
        if (!context_menu) {
            context_menu = new KiranWindowContextMenu(window);
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                        [this]() -> void {
                            this->get_toplevel()->hide();
                        });
        } else
            context_menu->refresh();

        context_menu->show_all();
        context_menu->popup_at_pointer(event);
    }

    if (window)
        window->activate(0);
    else
        g_warning("%s: window already deleted!!!\n", __PRETTY_FUNCTION__);
}

void KiranWindowPreviewer::on_close_button_clicked()
{
    auto window = get_window_();
    if (window) {
        window->close();
        signal_close().emit();
    }
}

void KiranWindowPreviewer::on_composite_changed()
{
    Gtk::Widget *snapshot_area = get_snapshot_area();
    if (is_composited()) {
        snapshot_area->show();
        get_style_context()->remove_class("vertical");
        get_style_context()->add_class("horizontal");
    } else {
        snapshot_area->set_visible(false);
        get_style_context()->remove_class("horizontal");
        get_style_context()->add_class("vertical");
    }

    if (get_realized())
        queue_resize();
}
