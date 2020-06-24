#include "kiran-tasklist-window-previewer.h"
#include <gtk/gtkx.h>
#include <cairomm/xlib_surface.h>
#include <iostream>

KiranWindowPreviewer::KiranWindowPreviewer(const std::shared_ptr<Kiran::Window> &window_):
    layout(Gtk::ORIENTATION_VERTICAL),
    top_layout(Gtk::ORIENTATION_HORIZONTAL),
    window(window_)
{
    std::shared_ptr<Kiran::Window> s_window = window.lock();
    icon_image.set_size_request(16, 16);
    if (s_window->get_icon()) {
        //take a copy here to avoid automatic free of pixbuf
        auto icon_pixbuf = Glib::wrap(s_window->get_icon(), true);

        auto real_pixbuf = icon_pixbuf->scale_simple(16, 16, Gdk::INTERP_BILINEAR);
        icon_image.set(real_pixbuf);
    }
    title_label.set_text(s_window->get_name());
    title_label.set_ellipsize(Pango::ELLIPSIZE_END);
    title_label.set_hexpand(true);
    title_label.set_halign(Gtk::ALIGN_START);

    //TODO, use a customed icon for close button
    close_btn.set_image_from_icon_name("gtk-close", Gtk::ICON_SIZE_BUTTON);
    //close_btn.set_relief(Gtk::RELIEF_NONE);
    close_btn.signal_clicked().connect([this]() -> void {
                                           if (!this->window.expired())
                                                this->window.lock()->close();
                                           //FIXME 如果窗口已经关闭?
                                       });

    signal_composited_changed().connect(sigc::mem_fun(*this, &KiranWindowPreviewer::on_composite_changed));

    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_no_show_all(true);
    auto context = get_style_context();
    context->add_class("window-previewer");
}

KiranWindowPreviewer::~KiranWindowPreviewer()
{
}

bool KiranWindowPreviewer::draw_snapshot(const Cairo::RefPtr<Cairo::Context> &cr)
{

    Gtk::Allocation allocation;
    double scale_x, scale_y, scale;
    Window xid;

    if (window.expired())
        return true;

    std::shared_ptr<Kiran::Window> s_window = window.lock();
    xid = s_window->get_xid();

    try {

        XWindowAttributes attrs;
        Display *xdisplay = GDK_DISPLAY_XDISPLAY(get_display()->gobj());

        XGetWindowAttributes(xdisplay, xid, &attrs);
        if (s_window->get_pixmap() == None)
            return false;


        auto surface = Cairo::XlibSurface::create(xdisplay,
                                                  s_window->get_pixmap(),
                                                  attrs.visual,
                                                  attrs.width,
                                                  attrs.height);

        if (!surface) {
            std::cerr<<"Failed to get pixbuf for window"<<std::endl;
            return false;
        }

        allocation = this->snapshot_area.get_allocation();
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
    minimum_width = natural_width = 240;
}

void KiranWindowPreviewer::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    if (is_composited()) {
        minimum_height = natural_height = 270;
    }
    else {
        layout.get_preferred_height(minimum_height, natural_height);
        g_message("natural height for window previewer is %d, %d\n", natural_height, minimum_height);
        minimum_height = natural_height;
    }
}

void KiranWindowPreviewer::on_realize()
{
    //g_message("realize window previewer 0x%x\n", this);
    auto close_box = Gtk::manage(new Gtk::EventBox());
    close_box->set_size_request(32, 32);

    close_box->add(close_btn);
    top_layout.add(icon_image);
    top_layout.add(title_label);
    top_layout.add(*close_box);
    top_layout.set_vexpand(false);
    top_layout.set_margin_start(10);
    top_layout.set_margin_end(10);
    top_layout.set_margin_top(10);
    top_layout.set_margin_bottom(10);
    top_layout.set_spacing(10);

    layout.add(top_layout);
    snapshot_area.set_size_request(200, 150);
    snapshot_area.set_hexpand(true);
    snapshot_area.set_vexpand(true);
    snapshot_area.set_halign(Gtk::ALIGN_CENTER);
    snapshot_area.set_valign(Gtk::ALIGN_CENTER);
    snapshot_area.signal_draw().connect(
                sigc::mem_fun(*this,&KiranWindowPreviewer::draw_snapshot));
    //layout.add(snapshot_area);
    add(layout);
    layout.show_all();

    close_btn.set_visible(false);

    on_composite_changed();
    Gtk::EventBox::on_realize();
}

bool KiranWindowPreviewer::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    close_btn.set_visible(true);

    if (get_state_flags() & Gtk::STATE_FLAG_PRELIGHT)
        return false;
    set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
    return false;
}

bool KiranWindowPreviewer::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    int root_x, root_y, width, height;
    Gtk::StateFlags flags;

    width = get_window()->get_width();
    height = get_window()->get_height();
    if (crossing_event->x > 0 && crossing_event->x <= width &&
            crossing_event->y > 0 && crossing_event->y <= height) {
        //鼠标仍旧在窗口范围内
        return false;
    }

    close_btn.set_visible(false);
    flags = get_state_flags();
    set_state_flags(flags & ~Gtk::STATE_FLAG_PRELIGHT, true);
    return false;
}

bool KiranWindowPreviewer::on_button_press_event(GdkEventButton *button_event)
{
    if (gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        //show context menu
        return false;
    }

    if (!this->window.expired())
        this->window.lock()->activate(button_event->time);
    else
        g_warning("on_button_press_event: window already deleted!!!\n");
}

void KiranWindowPreviewer::on_composite_changed()
{
    if (is_composited()) {
        snapshot_area.show();
    } else {

        snapshot_area.set_visible(false);
    }

    if (get_realized())
        queue_resize();
}
