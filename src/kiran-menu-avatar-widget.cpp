#include "kiran-menu-avatar-widget.h"
#include <iostream>

KiranMenuAvatarWidget::KiranMenuAvatarWidget(int size):
    icon_size(size)
{
    set_size_request(size, size);
    add_events(Gdk::BUTTON_PRESS_MASK);
    set_halign(Gtk::ALIGN_CENTER);
    set_valign(Gtk::ALIGN_CENTER);
}

void KiranMenuAvatarWidget::set_icon(const char *icon_)
{
    icon = icon_;
}

bool KiranMenuAvatarWidget::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    int radius;
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    allocation = get_allocation();
    radius = allocation.get_width() > allocation.get_height()?allocation.get_height():allocation.get_width();
    radius /= 2.0;
    try {
        if (!icon.empty())
            pixbuf = Gdk::Pixbuf::create_from_file(this->icon,
                                                   allocation.get_width(),
                                                   allocation.get_height());
        else
            pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-menu/icon/avatar",
                                                       allocation.get_width(),
                                                       allocation.get_height());
    } catch (const Glib::Error &e) {
        std::cerr<<"Failed to load avatar: "<<e.what()<<std::endl;
        return false;
    }

    cr->save();

    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->clip();
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->paint();

    //draw round borders
    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->set_source_rgba(1.0, 1.0, 1.0, 0.2);
    cr->set_line_width(1);
    cr->stroke();

    cr->restore();
    return false;
}

bool KiranMenuAvatarWidget::on_button_press_event(GdkEventButton *button_event)
{
    std::vector<std::string> args;

    args.push_back("/usr/bin/mate-about-me");
    args.push_back("");

    this->get_toplevel()->hide();
    Glib::spawn_async(std::string(), args, Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_CLOEXEC_PIPES);

    return false;
}
