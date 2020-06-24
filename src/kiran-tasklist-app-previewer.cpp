#include "kiran-tasklist-app-previewer.h"
#include <window-manager.h>

KiranAppPreviewer::KiranAppPreviewer():
    Gtk::Window(Gtk::WINDOW_POPUP),
    need_display(true),
    position(Gtk::POS_TOP),
    relative_to(NULL),
    is_idle(true)
{
    scroll_window.set_propagate_natural_height(true);
    scroll_window.set_propagate_natural_width(true);

    set_position(position, true);
    //signal_composited_changed().connect(sigc::mem_fun(*this, &KiranAppPreviewer::set_position));

    scroll_window.add(box);
    add(scroll_window);
    scroll_window.show_all();

    set_decorated(false);
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_no_show_all(true);

    box.set_margin_top(10);
    box.set_margin_bottom(10);
    box.set_margin_start(10);
    box.set_margin_end(10);
    box.set_spacing(10);
    box.signal_remove().connect(
                [this](Gtk::Widget *widget) -> void {
                    if (this->box.get_children().size() == 0)
                        this->hide();
                });
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
    return app;
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

        g_message("parent %p, geometry: (%d, %d), offset(%d, %d), size %d x %d, previewer size %d x %d\n",
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
            Gdk::Rectangle rect;
            auto display = get_display();

            auto monitor = display->get_monitor_at_window(
                        Glib::RefPtr<Gdk::Window>::cast_dynamic(get_window()));

            monitor->get_workarea(rect);
            if (new_x < rect.get_x())
                new_x = rect.get_x();
            else if (new_x > rect.get_x() + rect.get_width())
                new_x = rect.get_x() + rect.get_width();

            if (new_y < rect.get_y())
                new_y = rect.get_y();
            else if (new_y > rect.get_y() + rect.get_height())
                new_y = rect.get_y() + rect.get_height();

        } while (0);

        g_message("move previewer to (%d, %d)\n", new_x, new_y);
        move(new_x + 2, new_y);
    }
}

void KiranAppPreviewer::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    int tmp1, tmp2;
    Gdk::Rectangle rect;

    box.get_preferred_height(minimum_height, natural_height);
    auto monitor = box.get_display()->get_monitor_at_window(
                Glib::RefPtr<Gdk::Window>::cast_const(this->get_window()));

    //monitor->get_geometry(rect);
    monitor->get_workarea(rect);
    if (natural_height > 500) {
        //如果box的大小超过了当前显示器大小，使用滚动窗口大小，因为滚动窗口的natural_size包含了滚动条的size
        //scroll_window.get_preferred_height(minimum_height, natural_height);
        natural_height = 500;
    }

    minimum_height = natural_height;
}

void KiranAppPreviewer::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    int tmp1, tmp2;

    //scroll_window.get_vscrollbar()->get_preferred_width(tmp1, tmp2);
    //g_message("vscrollbar width %d, %d\n", minimum_width, natural_width);

    //box.get_preferred_width(minimum_width, natural_width);
    //g_message("box width %d, %d\n", minimum_width, natural_width);
    scroll_window.get_preferred_width(minimum_width, natural_width);
    //g_message("scrollwindow width %d, %d\n", minimum_width, natural_width);

    //minimum_width += tmp1 + 5;
    //natural_width += tmp2 + 5;
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
    set_idle(true);
    //schedule for hiding previewer
    Glib::signal_idle().connect(
                [this]() -> bool {
                    if (this->get_idle())
                        this->hide();
                    return false;
                });
    return false;
}

void KiranAppPreviewer::on_child_remove()
{
    g_message("after remove, %d children last\n", box.get_children().size());
    if (!box.get_children().size()) {
        //如果当前app没有已打开的窗口，隐藏预览窗口
        hide();
    }
}

void KiranAppPreviewer::load_windows_list()
{
    win_previewers.clear();
    for (auto child: box.get_children()) {
        box.remove(*child);
    }
    for (auto window: this->app->get_taskbar_windows()) {
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
