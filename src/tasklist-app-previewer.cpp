#include "tasklist-app-previewer.h"
#include <window-manager.h>

TasklistAppPreviewer::TasklistAppPreviewer():
    Gtk::Window(Gtk::WINDOW_POPUP),
    need_display(true),
    position(Gtk::POS_TOP),
    relative_to(nullptr),
    is_idle(true)
{
    set_rgba_visual();

    //signal_composited_changed().connect(sigc::mem_fun(*this, &KiranAppPreviewer::set_position));

    init_ui();

    set_decorated(false);
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_no_show_all(true);

    box.set_spacing(4);
    box.signal_remove().connect(
                [this](Gtk::Widget *widget) -> void {
                    if (this->box.get_children().size() == 0)
                        this->hide();
                });

    get_style_context()->add_class("app-previewer");
    //set_size_request(500, 500);
}


void TasklistAppPreviewer::set_app(const std::shared_ptr<Kiran::App> &app_)
{
    app = app_;
    load_windows_list();
    //TODO resize window
    adjust_size();
}

void TasklistAppPreviewer::set_idle(bool idle)
{
    is_idle = idle;
}

const std::shared_ptr<Kiran::App> TasklistAppPreviewer::get_app() const
{
    return app.lock();
}

bool TasklistAppPreviewer::get_idle() const
{
    return is_idle;
}

void TasklistAppPreviewer::set_relative_to(TasklistAppButton *widget, Gtk::PositionType pos)
{
    relative_to = widget;
    set_app(widget->get_app());
    set_position(pos);
}

void TasklistAppPreviewer::reposition()
{
    int parent_x, parent_y, new_x, new_y;
    Gtk::Requisition mini, natural;
    Gtk::Allocation parent_allocation;
    Gdk::Rectangle workarea;

    if (!relative_to)
        return;

    if (!get_realized())
        realize();

    auto relative_window = relative_to->get_window();
    relative_window->get_origin(parent_x, parent_y);
    parent_allocation = relative_to->get_allocation();

    g_debug("parent %p, geometry: (%d, %d), offset(%d, %d), size %d x %d, previewer size %d x %d\n",
            relative_to,
            parent_x, parent_y,
            parent_allocation.get_x(),
            parent_allocation.get_y(),
            parent_allocation.get_width(),
            parent_allocation.get_height(),
            natural.width,
            natural.height);

    if (!relative_to->get_has_window()) {
        //如果父控件没有自己的GdkWindow，那么需要重新计算其位置
        parent_x += parent_allocation.get_x();
        parent_y += parent_allocation.get_y();
    }

    get_preferred_size(mini, natural);
    switch (position) {
    case Gtk::POS_TOP:
        new_x = parent_x + (parent_allocation.get_width() - natural.width)/2;
        new_y = parent_y - natural.height;
        break;
    case Gtk::POS_BOTTOM:
        new_x = parent_x + (parent_allocation.get_width() - natural.width)/2;
        new_y = parent_y + parent_allocation.get_height();
        break;
    case Gtk::POS_LEFT:
        new_x = parent_x - natural.width;
        new_y = parent_y + (parent_allocation.get_height() - natural.height)/2;
        break;
    case Gtk::POS_RIGHT:
        new_x = parent_x + parent_allocation.get_width();
        new_y = parent_y + (parent_allocation.get_height() - natural.height)/2;
        break;
    }

    //检查位置是否超出当前显示器的工作区范围
    auto display = relative_to->get_display();
    auto monitor = display->get_monitor_at_window(relative_window);

    monitor->get_workarea(workarea);
    g_debug("%s: workarea (%d, %d), %d x %d, position %d x %d\n", __FUNCTION__,
            workarea.get_x(), workarea.get_y(),
            workarea.get_width(), workarea.get_height(),
            new_x, new_y);
    if (new_x < workarea.get_x())
        new_x = workarea.get_x() + 5;
    else
        new_x = std::min(new_x, workarea.get_x() + workarea.get_width());

    if (new_y < workarea.get_y())
        new_y = workarea.get_y() + 5;
    else
        new_y = std::min(new_y, workarea.get_y() + workarea.get_height());

    move(new_x, new_y);
}

void TasklistAppPreviewer::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    Gdk::Rectangle workarea;
    Gtk::Requisition min_size, natural_size;
    int min_width, natural_width;
    Glib::RefPtr<Gdk::Window> window;
    Glib::RefPtr<Gdk::Monitor> monitor;

    scroll_window.get_preferred_size(min_size, natural_size);
    if (relative_to == nullptr) {
        minimum_height = natural_height = 0;
        return;
    }

    window = relative_to->get_window();
    monitor = relative_to->get_display()->get_monitor_at_window(window);

    monitor->get_workarea(workarea);
    if (natural_size.height > workarea.get_height()) {
        //如果box的大小超过了当前显示器大小，使用滚动窗口大小，因为滚动窗口的natural_size包含了滚动条的size
        natural_height = workarea.get_height();
    } else {
        natural_height = natural_size.height;

        get_scrollbar()->get_preferred_width(min_width, natural_width);
        if (workarea.get_width() < natural_size.width) //需要额外加上滚动条的尺寸
            natural_height += natural_width;
    }

    minimum_height = natural_height;
}

void TasklistAppPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    Gdk::Rectangle workarea;
    Gtk::Requisition min_size, natural_size;
    Glib::RefPtr<Gdk::Window> window;
    Glib::RefPtr<Gdk::Monitor> monitor;

    if (relative_to == nullptr) {
        minimum_width = natural_width = 0;
        return;
    }
    window = relative_to->get_window();
    scroll_window.get_preferred_size(min_size, natural_size);

    monitor = relative_to->get_display()->get_monitor_at_window(window);
    monitor->get_workarea(workarea);
    if (workarea.get_width() < natural_size.width + 10) {
        natural_width = workarea.get_width() - 10;
    }
    else {
        int min_height, natural_height;

        natural_width = natural_size.width;
        get_scrollbar()->get_preferred_height(min_height, natural_height);
        if (workarea.get_height() < natural_size.height) //需要额外加上滚动条的尺寸
            natural_width += natural_height;
    }
    minimum_width = natural_width;
}

void TasklistAppPreviewer::adjust_size()
{
    Gtk::Requisition mini, natural;

    if (!get_realized())
        realize();

    //重新调整窗口大小
    get_preferred_size(mini, natural);
    if (natural.width > 0 && natural.height > 0)
        resize(natural.width, natural.height);
    reposition();
}

bool TasklistAppPreviewer::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    //阻止预览窗口隐藏
    set_idle(false);
    return false;
}

bool TasklistAppPreviewer::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    int width, height;

    width = get_window()->get_width();
    height = get_window()->get_height();
    if (crossing_event->x > 0 && crossing_event->x < width &&
            crossing_event->y > 0 && crossing_event->y < height) {
        //鼠标仍旧在窗口范围内
        return true;
    }

    set_idle(true);
    return false;
}

void TasklistAppPreviewer::on_child_remove()
{
    g_debug("after remove, %d children last\n", box.get_children().size());
    if (!box.get_children().size()) {
        //如果当前app没有已打开的窗口，隐藏预览窗口
        hide();
    }
}

void TasklistAppPreviewer::set_rgba_visual()
{
    //FIXME, 使用default_screen是否合适??
    auto visual = get_screen()->get_rgba_visual();
    if (!visual)
        g_warning("no rgba visual found\n");
    else
        gtk_widget_set_visual(reinterpret_cast<GtkWidget*>(this->gobj()), visual->gobj());
}

void TasklistAppPreviewer::init_ui()
{
    box.set_spacing(4);
    box.signal_remove().connect(
                sigc::hide(sigc::mem_fun(*this, &TasklistAppPreviewer::on_child_remove)));

    scroll_window.set_propagate_natural_height(true);
    scroll_window.set_propagate_natural_width(true);
    scroll_window.set_margin_top(5);
    scroll_window.set_margin_bottom(5);
    scroll_window.set_margin_start(5);
    scroll_window.set_margin_end(5);
    scroll_window.add(box);

    scroll_window.show_all();
    scroll_window.signal_scroll_event().connect(sigc::mem_fun(*this, &TasklistAppPreviewer::on_scroll_event));

    add(scroll_window);
}

void TasklistAppPreviewer::load_windows_list()
{
    auto app_ = get_app();

    win_previewers.clear();

    if (!app_) {
        g_warning("%s: app already expired\n", __FUNCTION__);
        return;
    }

    for (auto child: box.get_children()) {
        box.remove(*child);
        delete child;
    }
    for (auto window: app_->get_taskbar_windows()) {
        add_window_thumbnail(window);
    }
    box.show_all();
}

const Gtk::Scrollbar *TasklistAppPreviewer::get_scrollbar() const
{
    if (box.get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return scroll_window.get_hscrollbar();
    else
        return scroll_window.get_vscrollbar();
}

/**
 *
 */
void TasklistAppPreviewer::add_window_thumbnail(std::shared_ptr<Kiran::Window> &window)
{
    auto previewer = Gtk::manage(new TasklistWindowPreviewer(window));
    box.pack_start(*previewer, Gtk::PACK_SHRINK);
    previewer->show();

    auto data = std::make_pair(window->get_xid(), previewer);
    win_previewers.insert(data);

    //调整预览窗口大小和位置
    adjust_size();
}

void TasklistAppPreviewer::remove_window_thumbnail(std::shared_ptr<Kiran::Window> &window)
{
    auto iter = win_previewers.find(window->get_xid());
    if (iter == win_previewers.end())           //没有找到窗口的预览控件
        return;

    auto previewer = iter->second;
    box.remove(*previewer);
    win_previewers.erase(iter);
    adjust_size();
}

unsigned long TasklistAppPreviewer::get_thumbnails_count() {
    return win_previewers.size();
}

void TasklistAppPreviewer::set_position(Gtk::PositionType pos)
{
    Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL;

    if (position == pos && relative_to != nullptr)
        return;

    if (pos == Gtk::POS_LEFT || pos == Gtk::POS_RIGHT || !is_composited()) {
        /* 只有窗口管理器未开混成模式或者预览窗口位于左右侧时，窗口缩略图才需要纵向排列 */
        orient = Gtk::ORIENTATION_VERTICAL;
    }

    box.set_orientation(orient);

    if (orient == Gtk::ORIENTATION_HORIZONTAL)
        scroll_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);
    else
        scroll_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    reposition();
}

bool TasklistAppPreviewer::on_scroll_event(GdkEventScroll *event)
{
    /**
     * FIXME
     * 将滚动事件传递给滚动条，不知道为何没有自动传递滚动事件
     */
    Gtk::Scrollbar *scrollbar;

    scrollbar = const_cast<Gtk::Scrollbar*>(get_scrollbar());
    if (scrollbar->is_visible()) {
        GtkWidget *widget = reinterpret_cast<GtkWidget*>(scrollbar->gobj());
        gtk_propagate_event(widget, reinterpret_cast<GdkEvent*>(event));
     }
    return false;
}

