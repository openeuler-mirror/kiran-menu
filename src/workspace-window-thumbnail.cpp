#include "workspace-window-thumbnail.h"
#include "kiran-helper.h"
#include <gtk/gtkx.h>
#include <cairo/cairo-xlib.h>
#include "log.h"

WorkspaceWindowThumbnail::WorkspaceWindowThumbnail(KiranWindowPointer &win_, double scale_):
    Glib::ObjectBase("WorkspaceWindowSnapshot"),
    WindowThumbnailWidget (win_),
    border_width(4),
    thumbnail_surface(nullptr),
    scale(scale_),
    show_thumbnail(true)
{
    set_valign(Gtk::ALIGN_START);
    set_halign(Gtk::ALIGN_START);
    set_hexpand(false);
    set_vexpand(false);
    set_margin_top(10);
    set_vspacing(10);

    init_drag_and_drop();

    int scale_factor = get_scale_factor();
    window_width = WINDOW_WIDTH(win_);
    window_height = WINDOW_HEIGHT(win_);
    set_thumbnail_size(static_cast<int>(window_width/scale_factor  * scale),
                      static_cast<int>(window_height/scale_factor * scale));
}

WorkspaceWindowThumbnail::~WorkspaceWindowThumbnail()
{
    if (thumbnail_surface)
        cairo_surface_destroy(thumbnail_surface);
}

bool WorkspaceWindowThumbnail::draw_thumbnail_image(Gtk::Widget *area, const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto context = get_style_context();
    Gtk::Allocation allocation;
    auto window = get_window_();
    Cairo::ImageSurface *source_surface = nullptr;
    int scale_factor = get_scale_factor();
    
    if (!window) {
        LOG_WARNING("window expired\n");
        return true;
    }

    if (!generate_thumbnail())
        return false;

    allocation = area->get_allocation();

    source_surface = new Cairo::ImageSurface(thumbnail_surface, false);

    /**
     * 绘制窗口缩略图时不缩放
     */
    cr->save();
    cr->scale(1.0/scale_factor, 1.0/scale_factor);
    cr->set_source(Cairo::RefPtr<Cairo::Surface>(source_surface),
                   (allocation.get_width() * scale_factor - thumbnail_width)/2.0,
                   (allocation.get_height() * scale_factor - thumbnail_height)/2.0);
    cr->paint();
    cr->restore();

    if (get_state_flags() & Gtk::STATE_FLAG_PRELIGHT) {
        //绘制边框
        Gdk::RGBA color("#ff0000");
        Gdk::Rectangle rect;
        if (!context->lookup_color("thumbnail-hover-color", color)) {
            LOG_WARNING("Failed to load snapshot hover color from style files\n");
        }

        Gdk::Cairo::set_source_rgba(cr, color);
        cr->set_line_width(border_width);
        rect.set_x(border_width/2);
        rect.set_y(border_width/2);
        rect.set_width(allocation.get_width() - border_width);
        rect.set_height(allocation.get_height() - border_width);

        cr->rectangle(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());
        cr->stroke();
    }

    return true;
}

void WorkspaceWindowThumbnail::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{
    Gdk::Rectangle rect;
    auto window = get_window_();
    
    if (!window) {
        min_width = natural_width = 0;
        return;
    }

    min_width = 80;
    natural_width =  static_cast<int>(window_width/get_scale_factor()  * scale);
    if (natural_width < min_width)
        natural_width = min_width;
}

void WorkspaceWindowThumbnail::on_drag_data_delete(const Glib::RefPtr<Gdk::DragContext> &context)
{
    /* do nothing */
}

void WorkspaceWindowThumbnail::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::SelectionData &selection_data, guint info, guint time)
{
    //传递窗口的XID
    Window wid;
    auto window = get_window_();

    if (!window || selection_data.get_target() != "binary/XID") {
        gtk_drag_cancel(context->gobj());
        return;
    }

    wid = window->get_xid();
    selection_data.set("binary/XID", 8, reinterpret_cast<guint8*>(&wid), sizeof(wid));
}

void WorkspaceWindowThumbnail::on_drag_begin(const Glib::RefPtr<Gdk::DragContext> &context)
{
#define DRAG_THUMBNAIL_WIDTH 	250
#define DRAG_THUMBNAIL_HEIGHT 	200
    //设置拖动的图标为半透明的窗口截图
    Cairo::RefPtr<Cairo::ImageSurface> target_surface;
    Cairo::ImageSurface *source_surface = nullptr;
    Cairo::RefPtr<Cairo::Context> cr;
    double x_scale, y_scale, scale;

    x_scale = DRAG_THUMBNAIL_WIDTH * 1.0/thumbnail_width;
    y_scale = DRAG_THUMBNAIL_HEIGHT * 1.0/thumbnail_height;
    scale = std::min(x_scale, y_scale);

    source_surface = new Cairo::ImageSurface(thumbnail_surface, false);
    target_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
                                               thumbnail_width * scale,
                                               thumbnail_height * scale);
    cr = Cairo::Context::create(target_surface);
    cr->scale(scale, scale);
    cr->set_source(Cairo::RefPtr<Cairo::Surface>(source_surface), 0, 0);
    cr->paint_with_alpha(0.8);
    target_surface->flush();

    //将鼠标定位到拖动的图片中间位置
    target_surface->set_device_offset(0 - target_surface->get_width()/2.0,
                                      0 - target_surface->get_height()/2.0);
    context->set_icon(target_surface);
    show_thumbnail = false;
    queue_draw();
}

void WorkspaceWindowThumbnail::on_close_button_clicked()
{
    auto window = get_window_();
    if (window) {
        window->close();
    }
}

void WorkspaceWindowThumbnail::on_thumbnail_clicked()
{
    auto window = get_window_();
    if (window) {
        get_toplevel()->hide();
        window->activate(0);
    }
}

bool WorkspaceWindowThumbnail::generate_thumbnail()
{
    GdkDisplay *display;
    cairo_t *cr = nullptr;
    cairo_surface_t *surface = nullptr;
    int width, height, scale_factor;
    auto window = get_window_();


    if (!window)
        return false;

    if (thumbnail_surface != nullptr)
        cairo_surface_destroy(thumbnail_surface);

    display = get_display()->gobj();
    scale_factor = get_scale_factor();

    surface = window->get_thumbnail(width, height);
    if (surface == nullptr) {
        width = window_width;
        height = window_height;
    }
    thumbnail_width = static_cast<int>(width * scale);
    thumbnail_height = static_cast<int>(height* scale);
    thumbnail_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                   thumbnail_width,
                                                   thumbnail_height);

    cr = cairo_create(thumbnail_surface);
    if (surface != nullptr) {
        /**
         * 绘制缩放后的窗口缩略图
         */
        cairo_scale(cr, scale, scale);
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_paint(cr);
        cairo_surface_destroy(surface);
    } else {
        /**
         * 获取不到窗口截图，绘制窗口大小的阴影区域
         */
        cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
        cairo_paint(cr);
    }

    cairo_surface_flush(thumbnail_surface);
    cairo_destroy(cr);

    return true;
}

void WorkspaceWindowThumbnail::init_drag_and_drop()
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::TargetEntry entry("binary/XID", Gtk::TARGET_SAME_APP);

    targets.push_back(entry);
    drag_source_set(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);

    signal_drag_failed().connect(sigc::mem_fun(*this, &WorkspaceWindowThumbnail::on_drag_failed));
}


bool WorkspaceWindowThumbnail::on_drag_failed(const Glib::RefPtr< Gdk::DragContext > &context UNUSED,
                                              Gtk::DragResult result UNUSED)
{
    LOG_DEBUG("drag failed");
    show_thumbnail = true;
    queue_draw();
    return true;
}

bool WorkspaceWindowThumbnail::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!show_thumbnail)
        return false;

    return WindowThumbnailWidget::on_draw(cr);
}
