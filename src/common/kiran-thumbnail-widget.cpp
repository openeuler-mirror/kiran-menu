#include "kiran-thumbnail-widget.h"
#include "lib/base.h"

KiranThumbnailWidget::KiranThumbnailWidget() : show_close_button(true),
                                               show_icon_image(true)
{
    init_ui();
    get_style_context()->add_class("kiran-thumbnail");
}

void KiranThumbnailWidget::set_vspacing(int spacing)
{
    layout->set_row_spacing(spacing);
}

void KiranThumbnailWidget::set_title(const Glib::ustring &title_text)
{
    title_label->set_text(title_text);
}

void KiranThumbnailWidget::set_show_close_button(bool show)
{
    show_close_button = show;
    close_button->set_visible(show);
}

void KiranThumbnailWidget::set_show_icon(bool show)
{
    show_icon_image = show;
    icon_area->set_visible(show);
}

bool KiranThumbnailWidget::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    if (show_close_button)
    {
        close_button->show();
    }
    return Gtk::Button::on_enter_notify_event(crossing_event);
}

bool KiranThumbnailWidget::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    //隐藏关闭按钮
    if (crossing_event->detail != GDK_NOTIFY_INFERIOR && show_close_button)
    {
        close_button->hide();
    }

    return Gtk::Button::on_leave_notify_event(crossing_event);
}

void KiranThumbnailWidget::on_clicked()
{
    Gtk::Button::on_clicked();
    on_thumbnail_clicked();
}

/**
 * @brief 对关闭按钮的事件窗口重新定位
 * 
 * @note: 当前控件和关闭按钮都是按钮控件，按钮本身使用的都是parent的window，
 *        所以当前控件和关闭按钮的事件窗口的父窗口都是parent的窗口。而按钮
 *        事件都是是基于按钮的事件窗口(event window)，为了能让关闭按钮能正常
 *        获取到GDK事件，需要将关闭按钮的事件窗口的父窗口设置为当前控件的事件窗口,
 *        并根据父窗口的位置调整关闭按钮事件窗口的位置。
 */
void KiranThumbnailWidget::reposition_close_button()
{
    int x, y, child_x, child_y;

    if (!get_realized() || !close_button->get_realized())
        return;

    auto child_window = close_button->get_event_window();
    auto event_window = get_event_window();

    child_window->get_position(child_x, child_y);
    event_window->get_position(x, y);

    if (child_window->get_parent() == event_window)
        child_window->move(child_x - x, child_y - y);
    else
        child_window->reparent(event_window, child_x - x, child_y - y);
}

void KiranThumbnailWidget::init_ui()
{
    try
    {
        builder = Gtk::Builder::create_from_resource("/kiran-applet/ui/window-snapshot");
        builder->get_widget<Gtk::Label>("title-label", title_label);
        builder->get_widget<Gtk::Grid>("content-layout", layout);
        builder->get_widget<Gtk::DrawingArea>("icon-area", icon_area);
        builder->get_widget<Gtk::Button>("close-button", close_button);
        builder->get_widget<Gtk::DrawingArea>("snapshot-area", thumbnail_area);
    }
    catch (const Glib::Exception &e)
    {
        KLOG_ERROR("Error occured while trying to load ui file: %s\n", e.what().c_str());
        return;
    }

    close_button->set_no_show_all(true);
    close_button->hide();

    layout->reparent(*this);
    layout->show_all();

    close_button->signal_size_allocate().connect(
        sigc::hide(sigc::mem_fun(*this, &KiranThumbnailWidget::reposition_close_button)),
        true);

    close_button->signal_realize().connect(
        sigc::mem_fun(*this, &KiranThumbnailWidget::reposition_close_button),
        true);

    close_button->signal_clicked().connect(
        sigc::mem_fun(*this, &KiranThumbnailWidget::on_close_button_clicked));

    thumbnail_area->signal_draw().connect(
        sigc::bind<0, Gtk::Widget *>(sigc::mem_fun(*this, &KiranThumbnailWidget::draw_thumbnail_image), thumbnail_area),
        false);

    icon_area->signal_draw().connect(
        sigc::bind<0, Gtk::Widget *>(sigc::mem_fun(*this, &KiranThumbnailWidget::draw_icon_image), icon_area),
        false);
}

void KiranThumbnailWidget::set_thumbnail_size(int width, int height)
{
    thumbnail_area->set_size_request(width, height);
}

Gtk::Widget *KiranThumbnailWidget::get_thumbnail_area()
{
    return thumbnail_area;
}

bool KiranThumbnailWidget::draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    return false;
}