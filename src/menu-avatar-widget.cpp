#include "menu-avatar-widget.h"
#include "kiran-helper.h"
#include <iostream>

MenuAvatarWidget::MenuAvatarWidget(int size):
    icon_size(size)
{
    set_size_request(size, size);
    add_events(Gdk::BUTTON_PRESS_MASK);
    set_halign(Gtk::ALIGN_CENTER);
    set_valign(Gtk::ALIGN_CENTER);
}

void MenuAvatarWidget::set_icon(const Glib::ustring &icon_)
{
    icon = icon_;
}

void MenuAvatarWidget::set_icon_size(int size_) 
{
    if (icon_size == size_)
        return;
    icon_size = size_;
    set_size_request(icon_size, icon_size);
    queue_resize();
}

bool MenuAvatarWidget::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    int scale;
    double radius;
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    allocation = get_allocation();
	scale = get_scale_factor();
    radius = allocation.get_width() > allocation.get_height()?allocation.get_height():allocation.get_width();
    radius /= 2.0;
    try {
        pixbuf = Gdk::Pixbuf::create_from_file(icon,
                                               allocation.get_width() * scale,
                                               allocation.get_height() * scale);
    } catch (const Glib::Error &e) {
        g_warning("Failed to load avatar: %s", e.what().c_str());
        pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-menu/icon/avatar",
                                                   allocation.get_width() * scale,
                                                   allocation.get_height() * scale);
    }

    cr->save();

    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->clip();

	cr->scale(1.0/scale, 1.0/scale);
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->paint();

    //draw round borders
	cr->scale(1.0 * scale, 1.0 * scale);
    cr->arc(radius, radius, radius, 0, 2 * M_PI);
    cr->set_source_rgba(1.0, 1.0, 1.0, 0.2);
    cr->set_line_width(1);
    cr->stroke();

    cr->restore();
    return false;
}
