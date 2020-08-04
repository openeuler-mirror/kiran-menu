#include "kiran-tasklist-app-previewer.h"
#include <window-manager.h>

KiranAppPreviewer::KiranAppPreviewer():
    Gtk::Window(Gtk::WINDOW_POPUP),
    need_display(true),
    position(Gtk::POS_TOP),
    relative_to(nullptr),
    is_idle(true)
{
    set_rgba_visual();
    scroll_window.set_propagate_natural_height(true);
    scroll_window.set_propagate_natural_width(true);

    set_position(position, true);
    //signal_composited_changed().connect(sigc::mem_fun(*this, &KiranAppPreviewer::set_position));

    scroll_window.add(box);
    scroll_window.show_all();
    add(scroll_window);

    scroll_window.signal_scroll_event().connect(
                    [this](GdkEventScroll *event) -> bool {
                        /**
                         * FIXME
                         * 将滚动事件传递给滚动条，不知道为何没有自动传递滚动事件
                         */
                        Gtk::Scrollbar *scrollbar;

                        if (this->box.get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
                            scrollbar = this->scroll_window.get_hscrollbar();
                        else
                            scrollbar = this->scroll_window.get_vscrollbar();

                        if (scrollbar->is_visible())
                            gtk_propagate_event(GTK_WIDGET(scrollbar->gobj()),
                                            (GdkEvent*)event);
                        return false;
                    });

    set_decorated(false);
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_no_show_all(true);

    box.set_margin_start(5);
    box.set_margin_end(5);
    box.set_margin_top(5);
    box.set_margin_bottom(5);
    box.set_spacing(4);
    box.signal_remove().connect(
                [this](Gtk::Widget *widget) -> void {
                    if (this->box.get_children().size() == 0)
                        this->hide();
                });

    get_style_context()->add_class("app-previewer");
    //set_size_request(500, 500);
}


void KiranAppPreviewer::set_app(const std::shared_ptr<Kiran::App> &app_)
{
    app = app_;
    load_windows_list();
    //TODO resize window
    adjust_size();
}

void KiranAppPreviewer::set_idle(bool idle)
{
    is_idle = idle;
}

const std::shared_ptr<Kiran::App> KiranAppPreviewer::get_app() const
{
    return app.lock();
}

bool KiranAppPreviewer::get_idle() const
{
    return is_idle;
}

void KiranAppPreviewer::set_relative_to(Gtk::Widget *widget, Gtk::PositionType pos)
{
    relative_to = widget;
    set_position(pos);
}

void KiranAppPreviewer::reposition()
{
    int width, height;
    Gtk::Requisition mini, natural;

    if (!get_realized())
        realize();

    get_preferred_size(mini, natural);
    width = natural.width;
    height = natural.height;
    if (relative_to) {
        int parent_x, parent_y, new_x, new_y;
        Gtk::Allocation parent_allocation;

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

        switch (position) {
        case Gtk::POS_TOP:
            new_x = parent_x + (parent_allocation.get_width() - width)/2;
            new_y = parent_y - height;
            break;
        case Gtk::POS_BOTTOM:
            new_x = parent_x + (parent_allocation.get_width() - width)/2;
            new_y = parent_y + parent_allocation.get_height();
            break;
        case Gtk::POS_LEFT:
            new_x = parent_x - width;
            new_y = parent_y + (parent_allocation.get_height() - height)/2;
            break;
        case Gtk::POS_RIGHT:
            new_x = parent_x + parent_allocation.get_width();
            new_y = parent_y + (parent_allocation.get_height() - height)/2;
            break;
        }

        do {
            //检查位置是否超出当前显示器的工作区范围
            Gdk::Rectangle workarea;
            auto display = get_display();
            auto monitor = display->get_monitor_at_window(
                        Glib::RefPtr<Gdk::Window>::cast_dynamic(relative_window));

            monitor->get_workarea(workarea);
            g_debug("%s: workarea (%d, %d), %d x %d, position %d x %d\n", __FUNCTION__,
                    workarea.get_x(), workarea.get_y(),
                    workarea.get_width(), workarea.get_height(),
                    new_x, new_y);
            if (new_x < workarea.get_x())
                new_x = workarea.get_x();
            else if (new_x > workarea.get_x() + workarea.get_width())
                new_x = workarea.get_x() + workarea.get_width();

            if (new_y < workarea.get_y())
                new_y = workarea.get_y();
            else if (new_y > workarea.get_y() + workarea.get_height())
                new_y = workarea.get_y() + workarea.get_height();

        } while (0);

        g_debug("move previewer to (%d, %d)\n", new_x, new_y);
        move(new_x, new_y);
    }
}

void KiranAppPreviewer::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    Gdk::Rectangle workarea;
    Gtk::Requisition min_size, natural_size;
    int min_width, natural_width;

    scroll_window.get_preferred_size(min_size, natural_size);
    auto monitor = box.get_display()->get_monitor_at_window(
                Glib::RefPtr<Gdk::Window>::cast_const(this->get_window()));

    monitor->get_workarea(workarea);
    if (natural_size.height > workarea.get_height()) {
        //如果box的大小超过了当前显示器大小，使用滚动窗口大小，因为滚动窗口的natural_size包含了滚动条的size
        natural_height = workarea.get_height();
    } else {
        natural_height = natural_size.height;

        scroll_window.get_hscrollbar()->get_preferred_width(min_width, natural_width);
        if (workarea.get_width() < natural_size.width) //需要额外加上滚动条的尺寸
            natural_height += natural_width;
    }

    minimum_height = natural_height;
}

void KiranAppPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    Gdk::Rectangle workarea;
    Gtk::Requisition min_size, natural_size;

    scroll_window.get_preferred_size(min_size, natural_size);

    auto monitor = box.get_display()->get_monitor_at_window(
                Glib::RefPtr<Gdk::Window>::cast_const(this->get_window()));
    monitor->get_workarea(workarea);
    if (workarea.get_width() < natural_size.width)
        natural_width = workarea.get_width();
    else {
        int min_height, natural_height;

        natural_width = natural_size.width;
        scroll_window.get_hscrollbar()->get_preferred_height(min_height, natural_height);
        if (workarea.get_height() < natural_size.height) //需要额外加上滚动条的尺寸
            natural_width += natural_height;
    }
    minimum_width = natural_width;
}

void KiranAppPreviewer::adjust_size()
{
    Gtk::Requisition mini, natural;

    if (!get_realized())
        realize();

    //重新调整窗口大小
    get_preferred_size(mini, natural);
    if (natural.width > 0 && natural.height > 0)
        resize(natural.width, natural.height);
}

bool KiranAppPreviewer::on_enter_notify_event(GdkEventCrossing *crossing_event)
{
    //阻止预览窗口隐藏
    set_idle(false);
    return false;
}

bool KiranAppPreviewer::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    int width, height;

    width = get_window()->get_width();
    height = get_window()->get_height();
    if (crossing_event->x > 0 && crossing_event->x < width &&
            crossing_event->y > 0 && crossing_event->y < height) {
        //鼠标仍旧在窗口范围内
        return false;
    }

    set_idle(true);
    //schedule for hiding previewer
    Glib::signal_timeout().connect(
                [this]() -> bool {
                    if (this->get_idle())
                        this->hide();
                    return false;
                }, 300);
    return false;
}

/*
bool KiranAppPreviewer::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Widget *child = get_child();

    cr->save();
    cr->set_operator(Cairo::OPERATOR_CLEAR);
    //cr->set_source_rgb(0.0, 1.0, 0.0);
    cr->paint();
    cr->restore();

    propagate_draw(*child, cr);
    return false;
}
*/

void KiranAppPreviewer::on_child_remove()
{
    g_debug("after remove, %d children last\n", box.get_children().size());
    if (!box.get_children().size()) {
        //如果当前app没有已打开的窗口，隐藏预览窗口
        hide();
    }
}

void KiranAppPreviewer::on_show()
{
    Gtk::Window::on_show();

    signal_opened().emit();
}

void KiranAppPreviewer::set_rgba_visual()
{
    //FIXME, 使用default_screen是否合适??
    auto visual = get_screen()->get_rgba_visual();
    if (!visual)
        g_warning("no rgba visual found\n");
    else
        gtk_widget_set_visual(reinterpret_cast<GtkWidget*>(this->gobj()), visual->gobj());
}

void KiranAppPreviewer::load_windows_list()
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
        add_window_previewer(window);
    }
    box.show_all();
}

/**
 *
 */
void KiranAppPreviewer::add_window_previewer(const std::shared_ptr<Kiran::Window> &window, bool resize)
{
    if (window->get_window_type() != WNCK_WINDOW_NORMAL &&
        window->get_window_type() != WNCK_WINDOW_DIALOG)
        return;
    auto previewer = Gtk::manage(new KiranWindowPreviewer(window));
    box.pack_start(*previewer, Gtk::PACK_SHRINK);
    previewer->show();

    auto data = std::make_pair(window->get_xid(), previewer);
    win_previewers.insert(data);

    previewer->signal_close().connect(
                [window, this]() -> void {
                    //TODO 如何处理预览图移除后，鼠标移出预览窗口之外的问题?
                    this->remove_window_previewer(window);
                });

    if (resize) {
        //调整预览窗口大小和位置
        adjust_size();
        if (get_realized())
            reposition();
    }
}

void KiranAppPreviewer::remove_window_previewer(const std::shared_ptr<Kiran::Window> &window)
{
    if (window->get_window_type() != WNCK_WINDOW_NORMAL &&
        window->get_window_type() != WNCK_WINDOW_DIALOG)
        return;

    auto iter = win_previewers.find(window->get_xid());
    if (iter == win_previewers.end())           //没有找到窗口的预览控件
        return;

    auto previewer = iter->second;
    box.remove(*previewer);
    win_previewers.erase(iter);
    adjust_size();
    reposition();
}

uint32_t KiranAppPreviewer::get_previewer_num() {
    return win_previewers.size();
}

sigc::signal<void> KiranAppPreviewer::signal_opened()
{
    return m_signal_opened;
}


void KiranAppPreviewer::set_position(Gtk::PositionType pos, bool force)
{
    Gtk::Orientation orient = Gtk::ORIENTATION_VERTICAL;
    if (!force) {
            position = pos;
    }
    if (position == Gtk::POS_TOP || position == Gtk::POS_BOTTOM) {
        if (is_composited())
            orient = Gtk::ORIENTATION_HORIZONTAL;
    }
    box.set_orientation(orient);

    if (box.get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        scroll_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);
    else
        scroll_window.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    reposition();
}
