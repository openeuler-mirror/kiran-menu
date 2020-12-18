#include "window-thumbnail-widget.h"
#include "kiran-helper.h"


WindowThumbnailWidget::WindowThumbnailWidget(KiranWindowPointer &window_):
    window(window_)
{
    window_->signal_name_changed().connect(
        sigc::mem_fun(*this, &WindowThumbnailWidget::update_title));

    update_title();

    get_style_context()->add_class("window-thumbnail");
}

KiranWindowPointer WindowThumbnailWidget::get_window_() const
{
    return window.lock();
}

void WindowThumbnailWidget::update_title()
{
    if (window.expired())
        return;
    set_title(window.lock()->get_name());
}

bool WindowThumbnailWidget::draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    int scale_factor;
    GdkPixbuf *pixbuf = nullptr;

    if (window.expired())
        return false;

    scale_factor = get_scale_factor();
    pixbuf = window.lock()->get_icon();
    auto icon = Glib::wrap(pixbuf, true)->scale_simple(24 * scale_factor, 24 * scale_factor, Gdk::INTERP_BILINEAR);

    cr->scale(1.0/scale_factor, 1.0/scale_factor);
    Gdk::Cairo::set_source_pixbuf(cr, icon, 0, 0);
    cr->paint();
    return false;
}

bool WindowThumbnailWidget::draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    return false;
}
