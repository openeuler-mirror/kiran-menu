#include "workspace-thumbnail.h"
#include "window-manager.h"
#include "workspace-manager.h"
#include <gtk/gtkx.h>
#include <cairomm/xlib_surface.h>

WorkspaceThumbnail::WorkspaceThumbnail(KiranWorkspacePointer &workspace_) :
    Gtk::Box(Gtk::ORIENTATION_VERTICAL),
    workspace(workspace_),
    bg(nullptr),
    bg_surface(nullptr),
    is_current(false),
    drop_check(false),
    border_width(4)
{

    /*背景图片设置变化时重绘背景*/
    settings = Gio::Settings::create("org.mate.background");
    settings->signal_changed().connect(sigc::hide(sigc::mem_fun(*this, &WorkspaceThumbnail::redraw_background)));

    /*屏幕大小变化时重绘背景*/
    Gdk::Screen::get_default()->signal_size_changed().connect(sigc::mem_fun(*this, &WorkspaceThumbnail::redraw_background));

    set_spacing(10);
    pack_start(name_label, FALSE, FALSE);
    pack_end(snapshot_area, FALSE, FALSE);
    get_style_context()->add_class("workspace-previewer");

    name_label.set_xalign(0.0f);
    name_label.set_ellipsize(Pango::ELLIPSIZE_END);
    name_label.set_text(workspace_->get_name());
    snapshot_area.set_size_request(150, 80);
    snapshot_area.set_relief(Gtk::RELIEF_NONE);
    snapshot_area.signal_draw().connect(sigc::mem_fun(*this, &WorkspaceThumbnail::draw_snapshot));
    snapshot_area.get_style_context()->add_class("workspace-thumbnail");

    snapshot_area.signal_clicked().connect([this]() -> void {
                                                if (this->workspace.expired())
                                                    return;
                                                this->signal_selected().emit(this->workspace.lock()->get_number());
                                           });

    snapshot_area.signal_button_press_event().connect(
                [this](GdkEventButton *event) -> bool {
        if (this->workspace.expired()) {
            g_warning("workspace already expired");
            return false;
        }
        if (event->type == GDK_2BUTTON_PRESS) {
            /* Switch to workspace */
            this->workspace.lock()->activate(0);
            get_toplevel()->hide();
            return true;
        }
        return false;
    });

    init_drag_and_drop();
}

WorkspaceThumbnail::~WorkspaceThumbnail()
{
    if (bg_surface) {
        cairo_surface_destroy(bg_surface);
    }
}

void WorkspaceThumbnail::set_current(bool current)
{
    if (is_current == current)
        return;

    is_current = current;
    queue_draw();
}

bool WorkspaceThumbnail::get_is_current()
{
    return is_current;
}

sigc::signal<void, int> WorkspaceThumbnail::signal_selected()
{
    return m_signal_selected;
}

KiranWorkspacePointer WorkspaceThumbnail::get_workspace()
{
    return workspace.lock();
}

void WorkspaceThumbnail::redraw_background()
{
    reload_bg_surface();
    queue_draw();
}

void WorkspaceThumbnail::init_drag_and_drop()
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::TargetEntry entry("binary/XID", Gtk::TARGET_SAME_APP);

    targets.push_back(entry);
    snapshot_area.drag_dest_set(targets, Gtk::DEST_DEFAULT_HIGHLIGHT, Gdk::ACTION_MOVE);
    snapshot_area.signal_drag_motion().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) -> bool {
        g_debug("drag motion");
        this->drop_check = true;

        this->snapshot_area.drag_get_data(context, "binary/XID", time);
        return true;
    }, false);

    snapshot_area.signal_drag_drop().connect(
                [this](const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) -> bool {

        g_debug("drag-drop finished");
        this->drop_check = false;
        this->snapshot_area.drag_get_data(context, "binary/XID", time);
        return true;
    }, false);

    snapshot_area.signal_drag_data_received().connect(
        [this](const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection, guint info, guint time) -> void {
            Gdk::DragAction action = context->get_selected_action();
            int length;
            const guchar *data;
            const Window *wid;

            if (!drop_check && action != Gdk::ACTION_MOVE) {
                context->drag_finish(false, false, time);
                return;
            }

            data = selection.get_data(length);
            //传递的数据应该是Window的XID
            if (length < 0 || length != sizeof(Window)) {
                if (!this->drop_check)
                    context->drag_finish(false, false, time);
                else
                    context->drag_refuse(time);
                return;
            }

            wid = reinterpret_cast<const Window*>(data);

            auto window = Kiran::WindowManager::get_instance()->get_window(*wid);
            if (!window) {
                g_warning("Window with ID 0x%x not found\n", *wid);
                if (this->drop_check) {
                    context->drag_refuse(time);
                    return;
                }
                context->drag_finish(false, false, time);
                return;
            }

            if (window->get_workspace() == this->workspace.lock()) {
                if (this->drop_check) {
                    context->drag_refuse(time);
                    g_debug("Workspace of window and the target workspace are the same, drag-drop refused!");
                } else
                    context->drag_finish(false, false, time);
                return;
            }

            if (!this->drop_check) {
                //将对应的窗口移动到该工作区
                window->move_to_workspace(this->workspace.lock());
                context->drag_finish(true, true, time);
            } else {
                context->drag_status(Gdk::ACTION_MOVE, time);
            }
        });

}


bool WorkspaceThumbnail::reload_bg_surface()
{
    Gtk::Allocation allocation;
    double x_scale, y_scale;
    auto screen = get_screen();

    /**
     * 计算X和Y两个方向的缩放比例.
     * 为了应对多显示器扩展显示的情况，这里使用screen的大小
     */
    allocation = snapshot_area.get_allocation();
    surface_width = allocation.get_width() - 2 * border_width;
    surface_height = allocation.get_height() - 2 * border_width;
    x_scale = surface_width  * 1.0/screen->get_width();
    y_scale = surface_height * 1.0/screen->get_height();
    surface_scale = MIN(x_scale, y_scale);

    surface_width = static_cast<int>(screen->get_width() * surface_scale);
    surface_height = static_cast<int>(screen->get_height() * surface_scale);

    if (bg_surface)
        cairo_surface_destroy(bg_surface);

    bg = mate_bg_new();
    mate_bg_load_from_preferences(bg);

    if (!get_realized())
        return true;

    bg_surface = mate_bg_create_surface(bg,
                                        get_window()->gobj(),
                                        surface_width,
                                        surface_height,
                                        FALSE);
    g_object_unref(bg);
    return bg_surface != nullptr;
}

bool WorkspaceThumbnail::draw_snapshot(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto workspace = this->workspace.lock();
    Display *xdisplay = gdk_x11_get_default_xdisplay();
    Gtk::Allocation allocation;
    std::vector<Glib::RefPtr<Gdk::Window>> windows_stack;
    Cairo::ImageSurface *surface = nullptr;
    auto screen = get_screen();
    double surface_offset_x, surface_offset_y;
    int scale_factor = get_scale_factor();

    if (!workspace)
        return false;

    allocation = snapshot_area.get_allocation();
    //调用mate-desktop接口来获取桌面背景图片, 省去缩放等相关操作
    if (!bg_surface)
        reload_bg_surface();

    //背景图片居中显示
    surface_offset_x = (allocation.get_width() - surface_width)/2.0;
    surface_offset_y = (allocation.get_height() - surface_height)/2.0;
    surface = new Cairo::ImageSurface(bg_surface, false);
    cr->set_source(Cairo::RefPtr<Cairo::ImageSurface>(surface),
                   surface_offset_x,
                   surface_offset_y);
    cr->paint();

    cr->rectangle(surface_offset_x, surface_offset_y, surface_width, surface_height);
    cr->clip();

#if 1

    /**
     * 此处调用GDK接口来按照堆叠顺序获取窗口列表(靠底层的窗口在列表的靠前位置)，然后再进行过滤
     * 
     */
    for (auto gdk_window : screen->get_window_stack())
    {
        Kiran::WindowGeometry geometry;
        Window wid = GDK_WINDOW_XID(gdk_window->gobj());
        XWindowAttributes attrs;
        Gdk::Rectangle rect;

        int workspace_id = static_cast<int>(gdk_x11_window_get_desktop(gdk_window->gobj()));
        auto window = Kiran::WindowManager::get_instance()->get_window(wid);
        if (!window)
            continue;
        XGetWindowAttributes(xdisplay, wid, &attrs);

        /**
         *  仅绘制位于当前工作区或设置了所有工作区可见的非最小化窗口
         */
        if (window->is_minimized() || window->get_xid() == GDK_WINDOW_XID(get_toplevel()->get_window()->gobj()) ||
            (workspace->get_number() != workspace_id && !window->is_pinned()))
            continue;

        if (window->get_window_type() == WNCK_WINDOW_DOCK || window->get_window_type() == WNCK_WINDOW_DESKTOP)
            continue;

        KiranHelper::geometry_to_rect(window->get_geometry(), rect);
        Drawable drawable = window->get_pixmap();

        cr->save();
        cr->translate(surface_offset_x, surface_offset_y);
        cr->scale(surface_scale, surface_scale);

        /*window size from X server know nothing about scale*/
        if (drawable != None) {
            auto surface = Cairo::XlibSurface::create(xdisplay,
                                                  drawable,
                                                  attrs.visual,
                                                  attrs.width,
                                                  attrs.height);
            cr->scale(1.0/scale_factor, 1.0/scale_factor);
            cr->set_source(surface, rect.get_x(), rect.get_y());
            cr->paint();
        } else {
            auto icon_pixbuf = Glib::wrap(window->get_icon(), true);
            if (!icon_pixbuf)
                icon_pixbuf = Gtk::IconTheme::get_default()->load_icon(
                    "x-executable",
                    16,
                    scale_factor,
                    Gtk::ICON_LOOKUP_FORCE_SIZE);
            else
                icon_pixbuf = icon_pixbuf->scale_simple(16 * scale_factor, 16 * scale_factor, Gdk::INTERP_BILINEAR);

            cr->scale(1.0/scale_factor, 1.0/scale_factor);
            cr->set_source_rgba(0.0, 0.0, 0.0, 0.5);
            cr->rectangle(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());
            cr->fill();
            cr->restore();
            cr->save();

            if (icon_pixbuf) {
                double icon_offset_x, icon_offset_y;

                icon_offset_x = (rect.get_width() * surface_scale - icon_pixbuf->get_width()) / 2.0;
                icon_offset_y = (rect.get_height() * surface_scale - icon_pixbuf->get_height()) / 2.0;
                cr->translate(surface_offset_x, surface_offset_y);
                cr->scale(1.0/scale_factor, 1.0/scale_factor);
                Gdk::Cairo::set_source_pixbuf(cr,
                                              icon_pixbuf,
                                              rect.get_x() * surface_scale + icon_offset_x,
                                              rect.get_y() * surface_scale + icon_offset_y);
                cr->paint();
            }
        }
        cr->restore();
    }

#endif
    cr->reset_clip();
    if (is_current) {
        //FIXME, read color and border_width from css style
        /* 绘制选中后的边框 */
        Gdk::RGBA color("#ff0000");
        get_style_context()->lookup_color("thumbnail-hover-color", color);

        cr->set_line_width(border_width);
        Gdk::Cairo::set_source_rgba(cr, color);
        cr->rectangle(border_width/2.0,
                      border_width/2.0,
                      allocation.get_width() - border_width,
                      allocation.get_height() - border_width);
        cr->stroke();
    } else {
        cr->set_source_rgba(0.0, 0.0, 0.0, 0.3);
        cr->rectangle(border_width,
                      border_width,
                      allocation.get_width() - border_width * 2,
                      allocation.get_height() - border_width * 2);
        cr->fill();
    }

    return false;
}
