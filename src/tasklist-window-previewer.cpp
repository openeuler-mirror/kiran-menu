#include "tasklist-window-previewer.h"
#include <gtk/gtkx.h>
#include <cairomm/xlib_surface.h>
#include <iostream>
#include <glib/gi18n.h>

TasklistWindowPreviewer::TasklistWindowPreviewer(std::shared_ptr<Kiran::Window> &window_):
    WindowThumbnailWidget(window_),
    context_menu(nullptr),
    attention_color("red")
{
    needs_attention = window_->needs_attention();

    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);
    set_no_show_all(true);
    set_spacing(10);

    //如果窗口管理器混合模式关闭，我们需要隐藏窗口预览图，因为窗口预览图已经无法获取
    signal_composited_changed().connect(sigc::mem_fun(*this, &TasklistWindowPreviewer::on_composite_changed));


    window_state_change = window_->signal_state_changed().connect(
        sigc::mem_fun(*this, &TasklistWindowPreviewer::on_window_state_changed));

    /*
     * 从样式表中加载提示颜色
     */
    get_style_context()->add_class("window-previewer");
    if (!get_style_context()->lookup_color("tasklist_attention_color", attention_color))
    {
        g_warning("Failed to load attention-color from style");
    }

    on_composite_changed();
}

TasklistWindowPreviewer::~TasklistWindowPreviewer()
{
    if (context_menu)
        delete context_menu;

    if (window_state_change.connected())
        window_state_change.disconnect();
}

bool TasklistWindowPreviewer::draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    double scale_x, scale_y, scale;
    auto window = get_window_();
    int scale_factor = get_scale_factor();

    if (!window)
        return true;

    allocation = snapshot_area->get_allocation();
    try {
        int width, height;
        cairo_surface_t *thumbnail = window->get_thumbnail(width, height);
        if (thumbnail == nullptr) {
            //如果无法获取到窗口截图，同时窗口不可见，那么在预览区域绘制窗口图标
            int pixbuf_width, pixbuf_height;
            Glib::RefPtr<Gdk::Pixbuf> pixbuf;
            GdkPixbuf* c_pixbuf = window->get_icon();
            int icon_size = 64;

            if (!c_pixbuf) {
                auto icon_theme = Gtk::IconTheme::get_default();
                pixbuf = icon_theme->load_icon("application-x-executable",
                                               icon_size * scale_factor,
                                               Gtk::ICON_LOOKUP_FORCE_SIZE);
                return false;
            } else {
                pixbuf = Glib::wrap(c_pixbuf, true);
                pixbuf = pixbuf->scale_simple(icon_size * scale_factor,
                                              icon_size * scale_factor,
                                              Gdk::INTERP_BILINEAR);
            }

            pixbuf_width = pixbuf->get_width();
            pixbuf_height = pixbuf->get_height();

            cr->scale(1.0/scale_factor, 1.0/scale_factor);

            Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                          (allocation.get_width() * scale_factor - pixbuf_width)/2,
                                          (allocation.get_height() * scale_factor - pixbuf_height)/2);
            cr->paint();
            return false;
        } else {
            scale_x = allocation.get_width() * scale_factor * 1.0/width;
            scale_y = allocation.get_height()* scale_factor * 1.0/height;

            scale = std::min(scale_x, scale_y);
            cr->scale(1.0/scale_factor, 1.0/scale_factor);
            cr->translate((allocation.get_width() * scale_factor - width * scale)/2.0,
                          (allocation.get_height() * scale_factor - height * scale)/2.0);
            cr->scale(scale, scale);


            cairo_set_source_surface(cr->cobj(), thumbnail, 0, 0);
            cr->paint();
            cairo_surface_destroy(thumbnail);
        }
    } catch (const Glib::Error &e) {
        g_warning("Error occured while trying to draw window thumbnail: %s", e.what().c_str());
    }

    return false;
}

void TasklistWindowPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    GtkStateFlags flags;
    GValue value = G_VALUE_INIT;
    GtkWidget *widget = reinterpret_cast<GtkWidget*>(const_cast<GtkButton*>(gobj()));
    auto context = get_style_context()->gobj();

    flags = gtk_widget_get_state_flags(widget);
    gtk_style_context_get_property(context, "min-width", flags, &value);
    minimum_width = g_value_get_int(&value);
    g_value_unset(&value);

    natural_width = minimum_width;
}

bool TasklistWindowPreviewer::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto window = get_window_();

    if (!window)
        return false;

    if (window->needs_attention()) {
        /* 绘制需要注意的背景色 */
        auto child = get_child();

        Gdk::Cairo::set_source_rgba(cr, attention_color);
        cr->paint();
        propagate_draw(*child, cr);
        return false;
    }

    return WindowThumbnailWidget::on_draw(cr);
}

void TasklistWindowPreviewer::on_thumbnail_clicked()
{
    auto window = get_window_();

    if (window)
        window->activate(0);
    else
        g_warning("%s: window already deleted!!!\n", __PRETTY_FUNCTION__);
}

void TasklistWindowPreviewer::on_close_button_clicked()
{
    auto window = get_window_();
    if (window) {
        window->close();
    }
}

bool TasklistWindowPreviewer::on_button_press_event(GdkEventButton *button_event)
{
    GdkEvent *event;
    auto window = get_window_();

    if (G_UNLIKELY(!window))
        return false;

    event = reinterpret_cast<GdkEvent*>(button_event);
    if (gdk_event_triggers_context_menu(event)) {
        //show context menu
        if (!context_menu) {
            context_menu = new TasklistWindowContextMenu(window);
            context_menu->attach_to_widget(*this);
            context_menu->signal_deactivate().connect(
                        [this]() -> void {
                            signal_context_menu_toggled().emit(false);
                        });
        } else
            context_menu->refresh();

        context_menu->show_all();
        context_menu->popup_at_pointer(event);
        signal_context_menu_toggled().emit(true);
        return false;
    }

    return WindowThumbnailWidget::on_button_press_event(button_event);
}

void TasklistWindowPreviewer::on_composite_changed()
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

void TasklistWindowPreviewer::on_window_state_changed()
{
    auto window = get_window_();
    if (!window)
        return;

    if (needs_attention != window->needs_attention()) {
        needs_attention = window->needs_attention();
        queue_draw();
    }
}

bool TasklistWindowPreviewer::context_menu_is_opened()
{
    return context_menu != nullptr && context_menu->is_visible();
}

sigc::signal<void, bool> TasklistWindowPreviewer::signal_context_menu_toggled()
{
    return m_signal_context_menu_toggled;
}
