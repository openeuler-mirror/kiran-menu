#include "kiran-tasklist-window-previewer.h"
#include <gtk/gtkx.h>
#include <cairomm/xlib_surface.h>
#include <iostream>
#include <glib/gi18n.h>

KiranWindowPreviewer::KiranWindowPreviewer(const std::shared_ptr<Kiran::Window> &window_):
    layout(Gtk::ORIENTATION_VERTICAL),
    top_layout(Gtk::ORIENTATION_HORIZONTAL),
    window(window_),
    context_menu(nullptr)
{
    init_ui();
    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_no_show_all(true);

    //如果窗口管理器混合模式关闭，我们需要隐藏窗口预览图，因为窗口预览图已经无法获取
    signal_composited_changed().connect(sigc::mem_fun(*this, &KiranWindowPreviewer::on_composite_changed));

}

KiranWindowPreviewer::~KiranWindowPreviewer()
{
    if (context_menu)
        delete context_menu;

}

bool KiranWindowPreviewer::draw_snapshot(const Cairo::RefPtr<Cairo::Context> &cr)
{

    Gtk::Allocation allocation;
    double scale_x, scale_y, scale;
    Window xid;

    if (window.expired())
        return true;

    allocation = snapshot_area.get_allocation();
    std::shared_ptr<Kiran::Window> s_window = window.lock();
    xid = s_window->get_xid();

    try {
        Drawable drawable = s_window->get_pixmap();
        XWindowAttributes attrs;
        Display *xdisplay = GDK_DISPLAY_XDISPLAY(get_display()->gobj());

        XGetWindowAttributes(xdisplay, xid, &attrs);
        if (drawable == None) {
            if (attrs.map_state == IsViewable) {
                //无法从XComposite扩展获取到窗口截图，调用Xlib接口获取预览（仅针对非最小化的窗口)
                drawable = xid;
            } else {
                //如果无法获取到窗口截图，同时窗口不可见，那么在预览区域绘制窗口图标
                int pixbuf_width, pixbuf_height;
                GdkPixbuf* c_pixbuf = window.lock()->get_icon();
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
                                                  xid,
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

Gtk::SizeRequestMode KiranWindowPreviewer::get_request_mode_vfunc() const
{
    return Gtk::SIZE_REQUEST_CONSTANT_SIZE;
}

void KiranWindowPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    GtkStateFlags flags;
    GValue value = G_VALUE_INIT;
    GtkWidget *widget = reinterpret_cast<GtkWidget*>(const_cast<GtkEventBox*>(this->gobj()));
    auto context = get_style_context()->gobj();

    layout.get_preferred_width(minimum_width, natural_width);
    flags = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_property(context, "min-width", flags, &value);
    minimum_width = g_value_get_int(&value);
    g_value_unset(&value);

    natural_width = minimum_width;
}

void KiranWindowPreviewer::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{

    GtkStateFlags flags;
    GValue value = G_VALUE_INIT;
    GtkWidget *widget = reinterpret_cast<GtkWidget*>(const_cast<GtkEventBox*>(this->gobj()));
    auto context = get_style_context()->gobj();

    layout.get_preferred_height(minimum_height, natural_height);
    flags = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_property(context, "min-height", flags, &value);
    minimum_height = g_value_get_int(&value);
    g_value_unset(&value);

    if (natural_height < minimum_height)
        natural_height = minimum_height;

}

bool KiranWindowPreviewer::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (!close_btn.get_parent()) {
        top_layout.add(close_btn);
        close_btn.show_all();
    }

    if (get_state_flags() & Gtk::STATE_FLAG_PRELIGHT)
        return false;
    set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
    return false;
}

bool KiranWindowPreviewer::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    int width, height;
    Gtk::StateFlags flags;

    width = get_window()->get_width();
    height = get_window()->get_height();
    if (crossing_event->x > 0 && crossing_event->x <= width &&
            crossing_event->y > 0 && crossing_event->y <= height) {
        //右键菜单未打开，而且鼠标仍旧在窗口范围内
        if (!context_menu || !context_menu->get_mapped())
            return false;
    }

    top_layout.remove(close_btn);
    flags = get_state_flags();
    set_state_flags(flags & ~Gtk::STATE_FLAG_PRELIGHT, true);
    return false;
}

bool KiranWindowPreviewer::on_button_press_event(GdkEventButton *button_event)
{
    if (gdk_event_triggers_context_menu(reinterpret_cast<GdkEvent*>(button_event))) {
        //show context menu
        if (!context_menu) {
            context_menu = new KiranWindowContextMenu(window.lock());
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                        [this]() -> void {
                            this->get_toplevel()->hide();
                        });
        } else
            context_menu->refresh();

        context_menu->show_all();
        context_menu->popup_at_pointer(reinterpret_cast<GdkEvent*>(button_event));
        return false;
    }

    if (!this->window.expired())
        this->window.lock()->activate(button_event->time);
    else
        g_warning("%s: window already deleted!!!\n", __PRETTY_FUNCTION__);

    return false;
}

void KiranWindowPreviewer::on_composite_changed()
{
    if (is_composited()) {
        snapshot_area.show();
        get_style_context()->remove_class("vertical");
        get_style_context()->add_class("horizontal");
    } else {
        snapshot_area.set_visible(false);
        get_style_context()->remove_class("horizontal");
        get_style_context()->add_class("vertical");
    }

    if (get_realized())
        queue_resize();
}

void KiranWindowPreviewer::init_ui()
{
    auto s_window = window.lock();
    icon_image.set_size_request(16, 16);
    if (s_window->get_icon()) {
        //对底层的pixbuf增加引用，确保底层pixbuf不会随着C++对象销毁而被释放
        auto icon_pixbuf = Glib::wrap(s_window->get_icon(), true);

        auto real_pixbuf = icon_pixbuf->scale_simple(16, 16, Gdk::INTERP_BILINEAR);
        icon_image.set(real_pixbuf);
    }

    set_tooltip_text(s_window->get_name());
    title_label.set_text(s_window->get_name());
    title_label.set_ellipsize(Pango::ELLIPSIZE_END);
    title_label.set_hexpand(true);
    title_label.set_halign(Gtk::ALIGN_START);
    title_label.set_xalign(0.0);

    close_btn.get_style_context()->add_class("previewer-close-btn");
    close_btn.set_tooltip_text(_("Close window"));
    close_btn.set_relief(Gtk::RELIEF_NONE);
    close_btn.signal_clicked().connect([this]() -> void {
                                           if (!this->window.expired())
                                                this->window.lock()->close();
                                           //FIXME 如果窗口已经关闭?
                                       });

    top_layout.add(icon_image);
    top_layout.add(title_label);

    top_layout.set_vexpand(false);
    top_layout.set_spacing(10);

    snapshot_area.set_hexpand(true);
    snapshot_area.set_vexpand(true);
    snapshot_area.set_halign(Gtk::ALIGN_CENTER);
    snapshot_area.set_valign(Gtk::ALIGN_CENTER);
    snapshot_area.signal_draw().connect(
                sigc::mem_fun(*this,&KiranWindowPreviewer::draw_snapshot));

    snapshot_area.set_halign(Gtk::ALIGN_FILL);
    snapshot_area.set_valign(Gtk::ALIGN_FILL);

    layout.pack_start(top_layout, Gtk::PACK_SHRINK);
    layout.pack_start(snapshot_area, Gtk::PACK_EXPAND_WIDGET);

    layout.set_margin_start(8);
    layout.set_margin_end(8);
    layout.set_margin_top(8);
    layout.set_margin_bottom(8);
    layout.set_spacing(8);

    add(layout);
    layout.show_all();


    get_style_context()->add_class("window-previewer");

    on_composite_changed();
}
