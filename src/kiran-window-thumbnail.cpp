#include "kiran-window-thumbnail.h"
#include "kiranhelper.h"

KiranWindowThumbnail::KiranWindowThumbnail(KiranWindowPointer &window_):
    window(window_)
{
    try {
        builder = Gtk::Builder::create_from_resource("/kiran-applet/ui/window-snapshot");
        builder->get_widget<Gtk::Image>("icon-image", icon_image);
        builder->get_widget<Gtk::Label>("title-label", title_label);
        builder->get_widget<Gtk::Grid>("content-layout", layout);
        builder->get_widget<Gtk::DrawingArea>("close-area", close_area);
        builder->get_widget<Gtk::DrawingArea>("snapshot-area", snapshot_area);
    } catch (const Glib::Exception &e) {
        g_error("Error occured while trying to load ui file: %s\n", e.what().c_str());
        return;
    }

    init_ui();

    layout->reparent(*this);
    layout->show_all();

    add_events(Gdk::POINTER_MOTION_MASK);
    set_relief(Gtk::RELIEF_NONE);

    get_style_context()->add_class("window-previewer");

    signal_state_flags_changed().connect([this](Gtk::StateFlags flags) -> void {
        this->queue_draw();
    });

    window_->signal_name_changed().connect(sigc::mem_fun(*this, &KiranWindowThumbnail::update_title));
}

KiranWindowPointer KiranWindowThumbnail::get_window_() const
{
    return window.lock();
}

bool KiranWindowThumbnail::on_enter_notify_event(GdkEventCrossing* crossing_event)
{
    close_area->set_size_request(16, 16);
    close_area->queue_draw();
    set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
    return Gtk::Button::on_enter_notify_event(crossing_event);
}

bool KiranWindowThumbnail::on_leave_notify_event(GdkEventCrossing* crossing_event)
{
    //隐藏关闭按钮
    close_area->set_size_request(0, 0);
    return Gtk::Button::on_leave_notify_event(crossing_event);
}

bool KiranWindowThumbnail::on_motion_notify_event(GdkEventMotion *motion_event)
{
    Gtk::Allocation close_rect;
    GdkPoint point;

    close_rect = close_area->get_allocation();
    point.x = static_cast<int>(motion_event->x);
    point.y = static_cast<int>(motion_event->y);

    if (!get_has_window()) {
        /**
         * @brief GtkButton的事件基于其event_window，而close_area的尺寸基于button的window，该
         * window实际上是父窗口的window
         */
        Gtk::Allocation allocation;

        allocation = get_allocation();
        close_rect.set_x(close_rect.get_x() - allocation.get_x());
        close_rect.set_y(close_rect.get_y() - allocation.get_y());
    }

    /**
     * 检查鼠标是否经过关闭按钮区域
     */
    if (KiranHelper::gdk_rectangle_contains_point(close_rect.gobj(), &point)) {
        if (!(close_area->get_state_flags() & Gtk::STATE_FLAG_PRELIGHT)) {
            close_area->set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
            close_area->queue_draw();
        }
    } else{
        Gtk::StateFlags states = close_area->get_state_flags();
        if (states & Gtk::STATE_FLAG_PRELIGHT) {
            close_area->set_state_flags(states & ~Gtk::STATE_FLAG_PRELIGHT, true);
            close_area->queue_draw();
        }
    }

    return Gtk::Button::on_motion_notify_event(motion_event);
}

void KiranWindowThumbnail::on_clicked()
{
    GdkEvent *event = gtk_get_current_event();
    if (event->type == GDK_BUTTON_RELEASE) {
        Gtk::Allocation close_rect;
        Gtk::Allocation allocation;

        GdkPoint point;
        GdkEventButton *button_event = reinterpret_cast<GdkEventButton*>(event);

        allocation = get_allocation();
        close_rect = close_area->get_allocation();
        point.x = static_cast<int>(button_event->x);
        point.y = static_cast<int>(button_event->y);

        if (!get_has_window()) {
            /**
             * @brief GtkButton的事件基于其event_window，而close_area的尺寸基于button的window，该
             * window实际上是父窗口的window
             */

            close_rect.set_x(close_rect.get_x() - allocation.get_x());
            close_rect.set_y(close_rect.get_y() - allocation.get_y());
        }

        if (KiranHelper::gdk_rectangle_contains_point(close_rect.gobj(), &point)) {
            /**
             * @brief 如果鼠标释放时位于关闭区域内，当作点击关闭区域
             */
            on_close_button_clicked();
            return;
        }
    }
    Gtk::Button::on_clicked();
    on_thumbnail_clicked();
}

void KiranWindowThumbnail::on_close_button_clicked()
{
    g_message("close area clicked");
}

void KiranWindowThumbnail::on_thumbnail_clicked()
{
    g_message("thumbnail clicked");
}

bool KiranWindowThumbnail::draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation = snapshot_area->get_allocation();
    if (snapshot_area->get_has_window()) {
        allocation.set_x(0);
        allocation.set_y(0);
    }

    cr->rectangle(allocation.get_x() + 1,
                  allocation.get_y() + 1,
                  allocation.get_width() - 2,
                  allocation.get_height() - 2);
    cr->clip();
    cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->paint();

    if (get_state_flags() & Gtk::STATE_FLAG_PRELIGHT) {
        cr->reset_clip();
        cr->set_line_width(1.0);
        cr->set_source_rgb(0.0, 1.0, 0.0);
        cr->rectangle(allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height());
        cr->stroke();
    }

    return true;
}

bool KiranWindowThumbnail::draw_close_button(Gtk::Widget *close_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation = close_area->get_allocation();
    if (get_state_flags() & Gtk::STATE_FLAG_PRELIGHT)
    {
        //仅在鼠标经过窗口预览控件时显示关闭按钮
        std::string icon_name = "/kiran-tasklist/icon/close_normal";
        double x_offset, y_offset;
        int surface_size;

        if (close_area->get_state_flags() & Gtk::STATE_FLAG_PRELIGHT)
            icon_name = "/kiran-tasklist/icon/close_prelight";

        if (close_area->get_state_flags() & Gtk::STATE_FLAG_ACTIVE)
            icon_name = "/kiran-tasklist/icon/close_active";

        surface_size = MIN(allocation.get_width(), allocation.get_height());
        auto pixbuf = Gdk::Pixbuf::create_from_resource(icon_name);

        pixbuf = pixbuf->scale_simple(surface_size, surface_size, Gdk::INTERP_BILINEAR);

        //居中绘制
        x_offset = (allocation.get_width() - surface_size)/2.0;
        y_offset = (allocation.get_height() - surface_size)/2.0;
        Gdk::Cairo::set_source_pixbuf(cr, pixbuf, x_offset, y_offset);
        cr->paint();
    }
    return true;
}

void KiranWindowThumbnail::init_ui()
{
    snapshot_area->signal_draw().connect(
                sigc::bind<0, Gtk::Widget*>(sigc::mem_fun(*this, &KiranWindowThumbnail::draw_snapshot), snapshot_area), false);
    close_area->signal_draw().connect(
                sigc::bind<0, Gtk::Widget*>(sigc::mem_fun(*this, &KiranWindowThumbnail::draw_close_button), close_area), false);

    auto pixbuf = window.lock()->get_icon();
    if (pixbuf) {
        icon_image->set(Glib::wrap(pixbuf, true)->scale_simple(24, 24, Gdk::INTERP_BILINEAR));
    } else {
        auto pixbuf = Gtk::IconTheme::get_default()->load_icon("gtk-missing", 24);
        icon_image->set(pixbuf);
    }
    update_title();
}

void KiranWindowThumbnail::update_title()
{
    if (window.expired())
        return;
    title_label->set_text(window.lock()->get_name());
}

void KiranWindowThumbnail::set_snapshot_size(int width, int height)
{
    snapshot_area->set_size_request(width, height);
}
