#include "tasklist-buttons-container.h"
#include "global.h"
#include <window-manager.h>
#include <window.h>
#include "taskbar-skeleton.h"
#include "menu-skeleton.h"

#define PREVIEWER_ANIMATION_TIMEOUT 400

void on_applet_size_change(MatePanelApplet *applet UNUSED,
                           gint size UNUSED,
                           gpointer userdata)
{
    auto widget = reinterpret_cast<TasklistButtonsContainer*>(userdata);
    widget->handle_applet_size_change();
}

TasklistButtonsContainer::TasklistButtonsContainer(MatePanelApplet *applet_, int spacing_):
    applet(applet_),
    child_spacing(spacing_)
{
    init_ui();

    //响应窗口变化信号
    auto window_manager = Kiran::WindowManager::get_instance();
    window_manager->signal_active_window_changed().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_active_window_changed));
    window_manager->signal_window_opened().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_window_opened));
    window_manager->signal_window_closed().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_window_closed));

    //响应常住任务栏应用变化信号
    auto backend = Kiran::TaskBarSkeleton::get_instance();
    backend->signal_fixed_app_added().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_fixed_apps_added));
    backend->signal_fixed_app_deleted().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_fixed_apps_removed));

    auto menu_backend = Kiran::MenuSkeleton::get_instance();
    menu_backend->signal_app_changed().connect(sigc::mem_fun(*this, &TasklistButtonsContainer::reload_app_buttons));

    get_style_context()->add_class("tasklist-widget");

    add_events(Gdk::STRUCTURE_MASK);

    active_app = get_current_active_app();
}

TasklistButtonsContainer::~TasklistButtonsContainer()
{
    delete previewer;
}

/**
 * @brief KiranTasklistWidget::add_app_button
 *        为给定的应用创建应用按钮，并添加到任务栏. 如果应用对应的按钮在任务栏上已经存在，就直接返回
 * @param app 待添加的应用
 */
void TasklistButtonsContainer::add_app_button(const KiranAppPointer &app)
{
    int applet_size;
    TasklistAppButton *button = nullptr;

    if (!app || find_app_button(app))
        return;

    applet_size = static_cast<int>(mate_panel_applet_get_size(applet));

    button = Gtk::make_managed<TasklistAppButton>(app, applet_size);

    //鼠标进入应用按钮时，显示预览窗口
    button->signal_enter_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(
                        sigc::bind<TasklistAppButton*>(
                            sigc::mem_fun(*this, &TasklistButtonsContainer::move_previewer),
                            button)),
                    false));


    //鼠标点击时开关预览窗口
    button->signal_button_press_event().connect(
        [button, this](GdkEventButton *event) -> bool {
            if (gdk_event_triggers_context_menu(reinterpret_cast<GdkEvent*>(event)))
                return false;

            toggle_previewer(button);
            return false;
        });


    //鼠标离开应用按钮时，隐藏预览窗口
    button->signal_leave_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(sigc::mem_fun(*this, &TasklistButtonsContainer::hide_previewer)),
                    false));

    //应用右键菜单打开时，隐藏预览窗口
    button->signal_context_menu_toggled().connect(
                [this](bool active) -> void {
                    if (active)
                        hide_previewer();
                });

    add(*button);
    button->show_all();

    //建立app和应用按钮之间的映射
    app_buttons.insert(std::make_pair(app, button));
    g_debug("Add app button '%s'(%p)\n", app->get_name().data(), app.get());

}

void TasklistButtonsContainer::toggle_previewer(TasklistAppButton *button)
{
    auto target_app = button->get_app();
    auto previewer_app = previewer->get_app();

    if (!target_app) {
        g_warning("%s: target app expired\n", __FUNCTION__);
        return;
    }

    if (previewer_app && previewer_app == target_app && previewer->is_visible())
    {
        g_debug("previewer app and target app match\n");
        hide_previewer();
        return;
    } else
        move_previewer(button);
}

/**
 * @brief KiranTasklistWidget::remove_app_button 从任务栏上删除给定应用对应的应用按钮，如果应用按钮未找到，就直接返回
 * @param app 待删除的应用
 *
 */
void TasklistButtonsContainer::remove_app_button(const KiranAppPointer &app)
{
    if (!app)
        return;

    auto button = find_app_button(app);
    if (!button) {
        g_warning("%s: no button for app '%s'\n", __PRETTY_FUNCTION__, app->get_name().data());
        return;
    }

    g_debug("remove button for app '%s'\n", app->get_name().data());
    remove(*button);
    app_buttons.erase(app);

    delete button;
}

/**
 * @brief KiranTasklistWidget::get_current_active_app
 *        获取当前活动窗口所属的app
 *
 * @return 获取成功返回对应的app，否则返回空的shared_ptr
 */
KiranAppPointer TasklistButtonsContainer::get_current_active_app()
{
    auto active_window = Kiran::WindowManager::get_instance()->get_active_window();

    if (!active_window)
        return nullptr;
    else
        return active_window->get_app();
}

/**
 * @brief   KiranTasklistWidget::find_app_button
 *          在任务栏上查找给定应用对应的应用按钮
 * @param   app 待查找的应用
 * @return  成功返回app对应的应用按钮，否则返回nullptr
 */
TasklistAppButton *TasklistButtonsContainer::find_app_button(
        const KiranAppPointer &app)
{
    if (!app)
        return nullptr;

    auto iter = app_buttons.find(app);
    if (iter == app_buttons.end())
        return nullptr;

    return iter->second;
}

void TasklistButtonsContainer::on_active_window_changed(KiranWindowPointer previous,
                                                   KiranWindowPointer active)
{
    KiranAppPointer current_app = nullptr;
    TasklistAppButton *last_button, *active_button;

    if (!KiranHelper::window_is_ignored(active))
        current_app = active->get_app();

    active_button = find_app_button(current_app);
    if (current_app != active_app) {
        //active app changed

        g_debug("active app changed, new '%s', old '%s'\n",
                  current_app?current_app->get_name().data():"null",
                  active_app?active_app->get_name().data():"null");

        last_button = find_app_button(active_app);


        if (last_button) {
            last_button->queue_draw();
        }

        active_app = current_app;
    }


    if (active_button) {
        active_button->queue_draw();
        /*
         * 确保面板空间不足需要滚动时，活动窗口对应的应用按钮可见
         * 对于新打开的窗口，其应用按钮尚未初始化完成，该操作需要放在
         * 按钮的size_allocate信号处理函数中完成.
         */
         switch_to_page_of_button(active_button);
    }
}

void TasklistButtonsContainer::on_window_opened(KiranWindowPointer window)
{
    g_debug("window '%s' opened\n", window->get_name().data());

    /**
     * 如果窗口设置了不在工作区或任务栏上显示，就直接跳过
     */ 
    if (window->should_skip_taskbar())
        return;

    auto app = window->get_app();
    if (!app) {
        g_warning("no app found for new window '%s'\n", window->get_name().data());
        return;
    }

    auto app_button = find_app_button(app);
    if (!app_button) {
        add_app_button(app);
    }
    else {
        g_debug("app button already exists\n");
        app_button->set_has_tooltip(false);

        /**
         * 检查app的预览窗口是否打开，如果打开，需要更新预览窗口内容
         */
        if (previewer->is_visible() && previewer->get_app() == app) {
            previewer->add_window_thumbnail(window);
        } else
            app_button->queue_draw();
    }
}

void TasklistButtonsContainer::on_window_closed(KiranWindowPointer window)
{
    if (window->should_skip_taskbar())
        return;

    auto app = window->get_app();
    if (!app)
        return;

    if (previewer->is_visible() && previewer->get_app() == app) {
        //从预览窗口中删除该窗口的预览图
        previewer->remove_window_thumbnail(window);
    }

    auto app_button = find_app_button(app);
    if (app_button) {
        /**
         * 检查对应的应用是否有剩余窗口，如果没有，就删除该应用按钮
         */
        if (app->get_taskbar_windows().size() == 0) {
            //检查是否属于常驻任务栏的应用
            if (!KiranHelper::app_is_in_fixed_list(app))
                remove_app_button(app);
            else {
                app_button->set_tooltip_text(app->get_name());
                app_button->queue_draw();
            }
        } else
            app_button->set_has_tooltip(false);
    }
}

/**
 * @brief KiranTasklistWidget::move_previewer
 *        将预览窗口移动到指定的应用按钮位置，并显示应用按钮target_button所对应的app的窗口预览图.
 *        如果target_button对应的应用无已打开窗口，则隐藏预览窗口.
 * @param target_button 目标应用按钮
 */
void TasklistButtonsContainer::move_previewer(TasklistAppButton *target_button)
{
    Gtk::PositionType pos = Gtk::POS_BOTTOM;

    auto target_app = target_button->get_app();
    auto previewer_app = previewer->get_app();

    if (!target_app || target_app->get_taskbar_windows().size() == 0) {
        g_debug("target app expired or has no windows\n");
        return;
    }

    /* 当前预览窗口的右键菜单已经打开，不允许移动预览窗口 */
    if (previewer->has_context_menu_opened())
        return;

    previewer->set_idle(false);

    Glib::signal_timeout().connect_once([this, target_button]() -> void {
        auto target_app = target_button->get_app();
        auto previewer_app = previewer->get_app();
        if (previewer_app == target_app && previewer->is_visible()) {
            //当前预览的应用和目标应用是同一应用
            return;
        }
        if (previewer->get_idle()) {
            g_debug("previewer idle\n");
            return;
        }
        previewer->set_relative_to(target_button, get_previewer_position());
        previewer->show();
    }, PREVIEWER_ANIMATION_TIMEOUT);
}

void TasklistButtonsContainer::hide_previewer()
{

    if (previewer->has_context_menu_opened())
        return;
    previewer->set_idle(true);

    Glib::signal_timeout().connect_once(
                [this]() -> void {
                    if (previewer->get_idle()) {
                        previewer->hide();
                    }
                }, PREVIEWER_ANIMATION_TIMEOUT);
}

Gtk::PositionType TasklistButtonsContainer::get_previewer_position()
{
    Gtk::PositionType pos = Gtk::POS_TOP;

    if (applet) {
        switch (mate_panel_applet_get_orient(applet))
        {
        case MATE_PANEL_APPLET_ORIENT_DOWN:
            pos = Gtk::POS_BOTTOM;
            break;
        case MATE_PANEL_APPLET_ORIENT_UP:
            pos = Gtk::POS_TOP;
            break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
            pos = Gtk::POS_LEFT;
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            pos = Gtk::POS_RIGHT;
            break;
        }
    }

    return pos;
}

/**
 * @brief KiranTasklistWidget::update_orientation
 *        根据applet的排列方向设置任务栏应用按钮的排列方向
 */
void TasklistButtonsContainer::update_orientation()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment;
    if (!applet)
        return;

    switch (mate_panel_applet_get_orient(applet))
    {
    case MATE_PANEL_APPLET_ORIENT_DOWN:
    case MATE_PANEL_APPLET_ORIENT_UP:
        orient = Gtk::ORIENTATION_HORIZONTAL;
        adjustment = get_hadjustment();
        break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        orient = Gtk::ORIENTATION_VERTICAL;
        adjustment = get_vadjustment();
        break;
    }

    //FIXME 应该在此处重新连接信号???
//    if (adjustment)
//        adjustment->signal_value_changed().connect(
//                    [this]() -> void {
//                        signal_page_changed().emit();
//                    });

    if (get_realized()) {
        //调整应用按钮大小
        for (auto button: get_children()) {
            button->queue_resize();
        }
    }
}

Gtk::Orientation TasklistButtonsContainer::get_orientation() const
{
    return orient;
}

void TasklistButtonsContainer::handle_applet_size_change()
{
    int applet_size = get_applet_size();

    for (auto child: get_children()) {
        TasklistAppButton *button = dynamic_cast<TasklistAppButton*>(child);
        button->set_size(applet_size);
    }

    queue_resize();
}

int TasklistButtonsContainer::get_applet_size() const
{
    return static_cast<int>(mate_panel_applet_get_size(applet));
}


void TasklistButtonsContainer::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{

    if (orient == Gtk::ORIENTATION_HORIZONTAL) {
        int child_min_width, child_natural_width;
        auto children = get_children();

        if (children.size() == 0) {
            min_width = natural_width = 1;
            return;
        }

        auto child = children.at(0);
        child->get_preferred_width(child_min_width, child_natural_width);

        min_width = child_min_width;
        natural_width = (child_natural_width + child_spacing) * children.size() - child_spacing;
    } else {
        min_width = natural_width = get_applet_size();
    }
    //g_message("%s: container min %d, natural %d\n", __func__, min_width, natural_width);
}

void TasklistButtonsContainer::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{

    if (orient == Gtk::ORIENTATION_HORIZONTAL) {
        min_height = natural_height = get_applet_size();
    } else {
        int child_min_height, child_natural_height;
        auto children = get_children();

        if (children.size() == 0) {
            min_height = natural_height = 1;
            return;
        }

        auto child = children.at(0);
        child->get_preferred_height(child_min_height, child_natural_height);

        min_height = child_min_height;
        natural_height = (child_natural_height + child_spacing) * children.size() - child_spacing;
    }

}

void TasklistButtonsContainer::on_size_allocate(Gtk::Allocation &allocation)
{
    int child_total_size = 0, n_child = 0, child_index;
    int child_computed_size, child_min_size, child_max_size;
    int page_size;
    int real_child_spacing = child_spacing;
    std::vector<Gtk::Widget*> children = get_children();

    set_allocation(allocation);
    get_window()->move_resize(allocation.get_x(),
                              allocation.get_y(),
                              allocation.get_width(),
                              allocation.get_height());
    get_bin_window()->resize(allocation.get_width(),
                             allocation.get_height());

    n_child = children.size();
    if (n_child == 0)
        return;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        page_size = allocation.get_width();
    else
        page_size = allocation.get_height();
    child_computed_size = (page_size - (n_child - 1) * real_child_spacing)/n_child;

    /*
     * 由于所有child的尺寸都应该相同, 取第一个child的最小和最大尺寸即可
     */
    children.front()->get_preferred_width(child_min_size, child_max_size);

    if (child_computed_size >= child_min_size) {
        /* 放的下，不需要分页显示 */
        child_computed_size = std::min(child_computed_size, child_max_size);
        n_child_page = n_child;
    } else {
#if 0
        /*
         * 在child间隔不变的情况下，调整child尺寸，确保每页的child个数尽量多，同时剩余空间尽量小
         */
        int min_delta = G_MAXINT, best_width = child_min_size;
        for (child_computed_size = child_min_size; child_computed_size <= child_max_size; child_computed_size += 2) {
            int delta;

            n_child_page = (parent_size + child_spacing)/(child_computed_size + child_spacing);
            delta = (parent_size + child_spacing)%(child_computed_size + child_spacing);

            if (delta < min_delta) {
                best_width = child_computed_size;
                min_delta = delta;
            }
        }
        child_computed_size = best_width;
        n_child_page = (parent_size + child_spacing)/(child_computed_size + child_spacing);
#else
        /*
         * 计算每页能放的child的最大个数，同时调整child间距，使其排列更均匀
         */
        n_child_page = n_child;
        for (n_child_page = n_child - 1; n_child_page > 0; n_child_page--) {
            child_computed_size = (page_size - (n_child_page - 1) * real_child_spacing)/n_child_page;
            if (child_computed_size >= child_min_size) {
                real_child_spacing = (page_size - child_computed_size * n_child_page)/n_child_page-1;
                break;
            }
        }
#endif
    }

    child_index = 0;
    for (auto child: get_children()) {
        Gtk::Allocation child_allocation;

        if (child_index % n_child_page == 0) {
            /* 新的页面，child需要排在开头 */
            child_total_size = (child_index/n_child_page) * allocation.get_width();
        } else
            child_total_size += real_child_spacing;


        child_allocation.set_x(child_total_size);
        child_allocation.set_y(0);
        child_allocation.set_width(child_computed_size);
        child_allocation.set_height(allocation.get_height());
        child->size_allocate(child_allocation);

        child_total_size += child_computed_size;
        child_index++;
    }

    /* 确保滚动区域为整数个页面(可视区域)大小 */
    if (child_total_size % page_size != 0)
        child_total_size = (child_total_size / page_size + 1) * page_size;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        set_size(static_cast<unsigned int>(child_total_size),
                 static_cast<unsigned int>(allocation.get_height()));
    else
        set_size(static_cast<unsigned int>(allocation.get_width()),
                 static_cast<unsigned int>(child_total_size));
}

void TasklistButtonsContainer::on_add(Gtk::Widget *child)
{
    Gtk::Layout::on_add(child);
    queue_allocate();
}

void TasklistButtonsContainer::on_remove(Gtk::Widget *child)
{
    Gtk::Layout::on_remove(child);
    queue_allocate();
}

void TasklistButtonsContainer::on_map()
{
    Gtk::Layout::on_map();
    signal_page_changed().emit();
    //Schedule: 加载应用按钮
    Glib::signal_idle().connect_once(
                sigc::mem_fun(*this, &TasklistButtonsContainer::load_applications));
}

void TasklistButtonsContainer::on_realize()
{
    Gtk::Layout::on_realize();

    /*
     * 监控滚动区域变化(包括滚动区域本身和当前值)，大小变化和child变化都会导致滚动区域发生变化
     */
    auto adjustment = get_adjustment();
    adjustment->signal_changed().connect(
                [this]() -> void {
                    signal_page_changed().emit();
                    ensure_active_app_button_visible();
                });

    adjustment->signal_value_changed().connect(
                [this]() -> void {
                    signal_page_changed().emit();
                });
}

void TasklistButtonsContainer::reload_app_buttons()
{
    g_debug("%s: reloading apps\n", __FUNCTION__);

    for (auto iter: app_buttons) {
        auto app = iter.first;
        auto button = iter.second;

        delete button;
    }

    app_buttons.clear();

    load_applications();
}

/**
 * @brief KiranTasklistWidget::load_applications
 *        加载并显示应用按钮，具体包括常驻任务栏应用和已打开应用
 */
void TasklistButtonsContainer::load_applications()
{
    Kiran::AppVec apps;
    Kiran::TaskBarSkeleton *backend = Kiran::TaskBarSkeleton::get_instance();
    auto app_manager = Kiran::AppManager::get_instance();

    //加载常驻任务栏应用
    g_debug("%s: loading fixed apps ...\n", __FUNCTION__);
    apps = backend->get_fixed_apps();
    for (auto app: apps) {
        add_app_button(app);
    }

    //加载当前运行应用
    g_debug("%s: loading running apps ...\n", __FUNCTION__);
    apps = app_manager->get_running_apps();
    for (auto app: apps) {
        if (find_app_button(app)) {
            g_debug("button for app '%s' already exists, skip ...\n");
            continue;
        }

        //如果应用的窗口都不需要在任务栏显示，就不在任务栏上显示应用按钮
        if (app->get_taskbar_windows().size() == 0)
            continue;

        add_app_button(app);
    }
}

/**
 * @brief KiranTasklistWidget::on_fixed_apps_added
 *        常驻任务栏应用增加时的回调函数，
 * @param apps
 */
void TasklistButtonsContainer::on_fixed_apps_added(const Kiran::AppVec &apps)
{
    g_debug("Got fixed apps added signal\n");
    for (auto app: apps) {
        g_debug("%s: add fixed app '%s'\n", __func__,
                app->get_name().data());
        TasklistAppButton *button = find_app_button(app);
        if (!button) {
            //如果应用需要常驻任务栏，而且目前应用未打开，那就将其应用按钮添加到任务栏
           add_app_button(app);
        }
    }
}

void TasklistButtonsContainer::on_fixed_apps_removed(const Kiran::AppVec &apps)
{
    g_debug("Got fixed apps removed signal\n");
    for (auto app: apps) {
        g_debug("%s: remove fixed app '%s', %d windows found\n",
                __func__,
                app->get_name().data(),
                app->get_taskbar_windows().size());
        TasklistAppButton *button = find_app_button(app);

        if (button && app->get_taskbar_windows().size() == 0) {
            //如果应用已经不再常驻任务栏，而且无已打开窗口，那就将其应用按钮从任务栏上移除
            remove_app_button(app);
        }
    }
}

void TasklistButtonsContainer::on_previewer_window_opened()
{
    for (auto data: app_buttons) {
        auto button = data.second;
        button->on_previewer_opened();
    }
}

void TasklistButtonsContainer::switch_to_page_of_button(TasklistAppButton *button)
{
    Gtk::Allocation child_allocation;
    int view_size, offset_end;
    int page_no;

    if (button == nullptr)
        return;

    auto adjustment = get_adjustment();
    child_allocation = button->get_allocation();

    if (child_allocation.get_width() <= 1) {
        g_warning("%s: button not realized, allocation (%d, %d), %d x %d",
                  __func__,
                  child_allocation.get_x(),
                  child_allocation.get_y(),
                  child_allocation.get_width(),
                  child_allocation.get_height());
        return;
    }

    view_size = static_cast<int>(adjustment->get_page_size());

    if (orient == Gtk::ORIENTATION_HORIZONTAL) {
        offset_end = child_allocation.get_x() + child_allocation.get_width();
    }
    else {
        offset_end = child_allocation.get_y() + child_allocation.get_height();
    }

    page_no = offset_end / view_size;
    adjustment->set_value(page_no * view_size);
}

void TasklistButtonsContainer::ensure_active_app_button_visible()
{
    auto active_window = Kiran::WindowManager::get_instance()->get_active_window();

    if (!active_window)
        return;

    auto active_button = find_app_button(active_window->get_app());
    switch_to_page_of_button(active_button);
}

void TasklistButtonsContainer::init_ui()
{
    update_orientation();

    previewer = new TasklistAppPreviewer();
    //响应预览窗口打开信号
    previewer->signal_show().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_previewer_window_opened));
    previewer->signal_leave_notify_event().connect_notify(
                sigc::hide(sigc::mem_fun(*this, &TasklistButtonsContainer::hide_previewer)), true);
}

void TasklistButtonsContainer::move_to_next_page()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment;
    double value;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        adjustment = get_hadjustment();
    else
        adjustment = get_vadjustment();

    value = adjustment->get_value();
    value += adjustment->get_page_size();
    if (value + adjustment->get_page_size() > adjustment->get_upper())
        value = adjustment->get_upper();

    adjustment->set_value(value);
    queue_draw();
}

void TasklistButtonsContainer::move_to_previous_page()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment;
    double value;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        adjustment = get_hadjustment();
    else
        adjustment = get_vadjustment();

    //FIXME, 滚动整数个按钮大小?
    value = adjustment->get_value();
    value -= adjustment->get_page_size();
    if (value < adjustment->get_lower())
        value = adjustment->get_lower();

    adjustment->set_value(value);
    queue_draw();
}

bool TasklistButtonsContainer::has_previous_page()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment;
    double value;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        adjustment = get_hadjustment();
    else
        adjustment = get_vadjustment();

    value = adjustment->get_value();
    return value > adjustment->get_lower();
}

bool TasklistButtonsContainer::has_next_page()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment;
    double value;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        adjustment = get_hadjustment();
    else
        adjustment = get_vadjustment();

    value = adjustment->get_value();
    return value + adjustment->get_page_size() < adjustment->get_upper();
}

sigc::signal<void> TasklistButtonsContainer::signal_page_changed()
{
    return m_signal_page_changed;
}

Glib::RefPtr<Gtk::Adjustment> TasklistButtonsContainer::get_adjustment()
{
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return get_hadjustment();
    else
        return get_vadjustment();
}
