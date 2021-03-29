#include "tasklist-buttons-container.h"
#include "global.h"
#include <window-manager.h>
#include <workspace-manager.h>
#include "taskbar-skeleton.h"
#include "menu-skeleton.h"
#include "log.h"

#define PREVIEWER_ANIMATION_TIMEOUT 300

void on_applet_size_change(MatePanelApplet *applet UNUSED,
                           gint size UNUSED,
                           gpointer userdata)
{
    auto widget = reinterpret_cast<TasklistButtonsContainer*>(userdata);
    widget->on_applet_size_change();
}

TasklistButtonsContainer::TasklistButtonsContainer(MatePanelApplet *applet_, int spacing_):
    Glib::ObjectBase("KiranTasklistButtonsContainer"),
    applet(applet_),
    child_spacing(spacing_),
    m_property_orient(*this, "orientation", Gtk::ORIENTATION_HORIZONTAL),
    dragging_source(nullptr),
    dragging_icon(nullptr)
{
    init_ui();

    g_signal_connect_after(applet_, "change-size", G_CALLBACK(::on_applet_size_change), this);

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

    /* 响应应用按钮显示策略变化 */
    backend->signal_app_show_policy_changed().connect(
        sigc::mem_fun(*this, &TasklistButtonsContainer::on_app_show_policy_changed));

    /* 当前活动工作区变化后，重新加载应用按钮 */
    auto workspace_manager = Kiran::WorkspaceManager::get_instance();
    workspace_manager->signal_active_workspace_changed().connect(
        sigc::mem_fun(*this, &TasklistButtonsContainer::on_active_workspace_changed));

    /* 后台应用数据刷新后，需要重新加载数据 */
    auto menu_backend = Kiran::MenuSkeleton::get_instance();
    menu_backend->signal_app_changed().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::load_applications));

    /* 响应任务栏面板位置变化 */
    property_orient().signal_changed().connect(
                sigc::mem_fun(*this, &TasklistButtonsContainer::on_orientation_changed));

    get_style_context()->add_class("tasklist-widget");

    add_events(Gdk::STRUCTURE_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);

    active_app = get_current_active_app();

    init_dnd();
}

TasklistButtonsContainer::~TasklistButtonsContainer()
{
    stop_pointer_check();
    delete previewer;
}

Glib::PropertyProxy<Gtk::Orientation> TasklistButtonsContainer::property_orient()
{
    return m_property_orient.get_proxy();
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

    button->signal_enter_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(sigc::mem_fun(*this, &TasklistButtonsContainer::schedule_pointer_check)),
                    false));
    button->signal_leave_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(sigc::mem_fun(*this, &TasklistButtonsContainer::schedule_pointer_check)),
                    false));

    button->signal_drag_update().connect(
        sigc::bind<0>(
            sigc::mem_fun(*this, &TasklistButtonsContainer::on_button_drag_motion),
            button));

    button->signal_drag_end().connect(
        sigc::bind<0>(
            sigc::mem_fun(*this, &TasklistButtonsContainer::on_button_drag_end),
            button));

    //鼠标点击时开关预览窗口
    button->signal_clicked().connect(
        [button, this]() -> void {
            stop_pointer_check();

            auto target_app = button->get_app();
            auto previewer_app = previewer->get_app();

            if (previewer_app && previewer_app == target_app && previewer->is_visible())
                hide_previewer();
            else
                move_previewer(button);
        });


    //应用右键菜单打开时，隐藏预览窗口
    button->signal_context_menu_toggled().connect(
                [this](bool active) -> void {
                    if (!active)
                        return;
                    stop_pointer_check();
                    hide_previewer();
                });

    add(*button);
    button->show_all();

    //建立app和应用按钮之间的映射
    app_buttons.insert(std::make_pair(app, button));
    LOG_DEBUG("add button for app '%s'", app->get_desktop_id().c_str());
}

void TasklistButtonsContainer::schedule_pointer_check()
{
    if (!pointer_check.connected()) {
        pointer_check = Glib::signal_timeout().connect(
                    sigc::bind_return<bool>(
                        sigc::mem_fun(*this, &TasklistButtonsContainer::check_and_toggle_previewer),
                        false),
                    PREVIEWER_ANIMATION_TIMEOUT);
    }
}

void TasklistButtonsContainer::stop_pointer_check()
{
    pointer_check.disconnect();
}

void TasklistButtonsContainer::check_and_toggle_previewer()
{
    GdkPoint point;
    auto pointer_device = get_display()->get_default_seat()->get_pointer();

    pointer_device->get_position(point.x, point.y);

    if (dragging_source != nullptr) {
        /* 当前正进行拖动操作，不显示预览窗口 */
        hide_previewer();
        return;
    }

    if (previewer->contains_pointer()) {
        /*
         * 鼠标位于预览窗口内
         */
        return;
    }

    for (auto data: app_buttons) {
        GdkRectangle rect;
        Gdk::Rectangle geometry;
        auto button = data.second;

        if (!button->is_visible())
            continue;

        get_child_geometry(static_cast<Gtk::Widget*>(button), geometry);
        rect.x = geometry.get_x();
        rect.y = geometry.get_y();
        rect.width = geometry.get_width();
        rect.height = geometry.get_height();

        if (KiranHelper::gdk_rectangle_contains_point(&rect, &point)) {
            /*
             * 鼠标位于某个应用按钮的上方，将应用预览窗口移动到对应的应用按钮处
             */
            move_previewer(button);
            return;
        }
    }

    /*鼠标位于任务栏应用按钮之外，直接隐藏预览窗口*/
    hide_previewer();
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
        LOG_WARNING("no button for app '%s'", app->get_name().data());
        return;
    }

    LOG_DEBUG("remove button for app '%s'", app->get_name().data());
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

        LOG_DEBUG("active app changed, new '%s', old '%s'\n",
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

void TasklistButtonsContainer::on_active_workspace_changed(std::shared_ptr<Kiran::Workspace> last_active,
                                     std::shared_ptr<Kiran::Workspace> active)
{
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    if (backend->get_app_show_policy() == Kiran::TaskBarSkeleton::POLICY_SHOW_ACTIVE_WORKSPACE) {
        load_applications();
    }
}

void TasklistButtonsContainer::on_window_opened(KiranWindowPointer window)
{
    auto backend = Kiran::TaskBarSkeleton::get_instance();
    LOG_DEBUG("window '%s' opened\n", window->get_name().data());

    /*
     * 如果窗口设置了不在工作区或任务栏上显示，就直接跳过
     */ 
    if (window->should_skip_taskbar())
        return;

    if (backend->get_app_show_policy() != Kiran::TaskBarSkeleton::POLICY_SHOW_ALL &&
        !KiranHelper::window_is_on_active_workspace(window))
        return;

    auto app = window->get_app();
    if (!app) {
        LOG_WARNING("no app found for new window '%s'\n", window->get_name().data());
        return;
    }

    auto app_button = find_app_button(app);
    if (!app_button) {
        add_app_button(app);
    }
    else {
        LOG_DEBUG("app button already exists\n");
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

    auto backend = Kiran::TaskBarSkeleton::get_instance();
    if (backend->get_app_show_policy() != Kiran::TaskBarSkeleton::POLICY_SHOW_ALL &&
        !KiranHelper::window_is_on_active_workspace(window))
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
        if (KiranHelper::get_taskbar_windows(app).size() == 0) {
            //检查是否属于常驻任务栏的应用
            if (!KiranHelper::app_is_in_fixed_list(app))
                remove_app_button(app);
            else {
                app_button->set_tooltip_text(app->get_name());
                app_button->queue_draw();
            }
        } else {
            app_button->set_has_tooltip(false);
            app_button->queue_draw();
        }
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
    auto target_app = target_button->get_app();
    auto previewer_app = previewer->get_app();


    /* 当前预览窗口的右键菜单已经打开，不允许移动预览窗口 */
    if (previewer->has_context_menu_opened())
        return;

    if (!target_app || KiranHelper::get_taskbar_windows(target_app).size() == 0) {
        LOG_DEBUG("target app expired or has no windows\n");
        previewer->hide();
        return;
    }

    if (previewer->get_app() == target_button->get_app() && previewer->is_visible()) {
        //当前预览的应用和目标应用是同一应用
        return;
    }
    previewer->set_relative_to(target_button, get_previewer_position());
    previewer->show();
}

void TasklistButtonsContainer::hide_previewer()
{
    if (previewer->has_context_menu_opened())
        return;
    previewer->hide();
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
    if (!applet)
        return;

    switch (mate_panel_applet_get_orient(applet))
    {
    case MATE_PANEL_APPLET_ORIENT_DOWN:
    case MATE_PANEL_APPLET_ORIENT_UP:
        m_property_orient.set_value(Gtk::ORIENTATION_HORIZONTAL);
        break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        m_property_orient.set_value(Gtk::ORIENTATION_VERTICAL);
        break;
    }
}

Gtk::Orientation TasklistButtonsContainer::get_orientation() const
{
    return m_property_orient.get_value();
}

void TasklistButtonsContainer::on_applet_size_change()
{
    int applet_size = get_applet_size();

    LOG_DEBUG("applet size changed to %d", applet_size);
    for (auto child: get_children()) {
        TasklistAppButton *button = dynamic_cast<TasklistAppButton*>(child);
        button->set_size(applet_size);
    }

    queue_allocate();
}

int TasklistButtonsContainer::get_applet_size() const
{
    return static_cast<int>(mate_panel_applet_get_size(applet));
}


void TasklistButtonsContainer::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
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
    //LOG_MESSAGE("container min %d, natural %d\n", min_width, natural_width);
}

void TasklistButtonsContainer::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
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

    n_child = children.size();
    if (n_child == 0) {

        /* gtk_layout_set_size()会自动调整bin_window大小 */
        set_size(allocation.get_width(), allocation.get_height());
        return;
    }

    /*
     * 由于所有child的尺寸都应该相同, 尺寸信息取第一个child的最小和最大尺寸即可
     */
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        page_size = allocation.get_width();
        children.front()->get_preferred_width(child_min_size, child_max_size);
    }
    else {
        page_size = allocation.get_height();
        children.front()->get_preferred_height(child_min_size, child_max_size);
    }

    child_computed_size = (page_size - (n_child - 1) * real_child_spacing)/n_child;

    if (child_computed_size >= child_min_size) {
        /* 放的下，不需要分页显示 */
        child_computed_size = std::min(child_computed_size, child_max_size);
        n_child_page = n_child;
    } else {
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
    }

    child_index = 0;

    /*
     * 将child按照其在父控件上的位置从前到后进行排序。
     *
     * child拖动之后其在父控件上的位置会发生变化。
     */
    std::sort(children.begin(), children.end(),
              [this](const Gtk::Widget *c1, const Gtk::Widget *c2) -> bool {
                int x1, x2, y1, y2;

                x1 = child_property_x(*c1).get_value();
                x2 = child_property_x(*c2).get_value();
                y1 = child_property_y(*c1).get_value();
                y2 = child_property_y(*c2).get_value();

                if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
                    return x1 != x2 ?(x1 < x2):(y1 < y2);
                else
                    return y1 != y2 ?(y1 < y2):(x1 < x2);
              });

    for (auto child: children) {
        Gtk::Allocation child_allocation;

        if (child_index % n_child_page == 0) {
            /* 新的页面，child需要排在开头 */
            child_total_size = (child_index/n_child_page) * page_size;
        } else
            child_total_size += real_child_spacing;

        if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
            child_allocation.set_x(child_total_size);
            child_allocation.set_y(0);
            child_allocation.set_width(child_computed_size);
            child_allocation.set_height(allocation.get_height());
            child_property_x(*child).set_value(child_total_size);
            child_property_y(*child).set_value(0);
        } else {
            child_allocation.set_x(0);
            child_allocation.set_y(child_total_size);
            child_allocation.set_width(allocation.get_width());
            child_allocation.set_height(child_computed_size);
            child_property_x(*child).set_value(0);
            child_property_y(*child).set_value(child_total_size);
        }
        child->size_allocate(child_allocation);

        child_total_size += child_computed_size;
        child_index++;
    }

    /* 确保滚动区域为整数个页面(可视区域)大小 */
    if (child_total_size % page_size != 0)
        child_total_size = (child_total_size / page_size + 1) * page_size;

    /* 对GtkLayout调用set_size()时，bin_window会自动做resize */
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
    /* 确保新添加的child在size_allocate的时候，排在最后 */
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        child_property_x(*child).set_value(G_MAXINT);
    else
        child_property_y(*child).set_value(G_MAXINT);
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

    init_paging_monitor();
}

void TasklistButtonsContainer::on_unrealize()
{
    if (adjustment_changed.connected())
        adjustment_changed.disconnect();

    if (paging_notify.connected())
        paging_notify.disconnect();

    Gtk::Layout::on_unrealize();
}

bool TasklistButtonsContainer::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    /* 绘制背景 */
    get_style_context()->render_background(cr,
                                           0,
                                           0,
                                           get_allocated_width(),
                                           get_allocated_height());

    for (auto child : get_children())
    {
        /* 对于正处于拖动过程中的按钮不进行绘制 */
        if (dragging_source != nullptr && child == dragging_source)
            continue;

        propagate_draw(*child, cr);
    }

    /* 绘制拖动的按钮缩略图 */
    if (dragging_source != nullptr) {
        int icon_x, icon_y;
        Gtk::Allocation allocation, child_allocation;

        allocation = get_allocation();
        child_allocation = dragging_source->get_allocation();

        /*
         * 确保缩略图绘制不会超出应用按钮显示区域
         */
        icon_x = std::max(0, dragging_pos.get_x());
        icon_x = std::min(icon_x, allocation.get_x() + allocation.get_width() - child_allocation.get_width());

        icon_y = std::max(0, dragging_pos.get_y());
        icon_y = std::min(icon_y, allocation.get_y() + allocation.get_height() - child_allocation.get_height());

        Gdk::Cairo::set_source_pixbuf(cr,
                                      dragging_icon,
                                      icon_x,
                                      icon_y);
        cr->paint_with_alpha(0.7);
    }

    return false;
}

void TasklistButtonsContainer::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context,
                                                     int x,
                                                     int y,
                                                     const Gtk::SelectionData &selection_data,
                                                     guint info,
                                                     guint time)
{
    auto app_manager = Kiran::AppManager::get_instance();
    for (auto uri: selection_data.get_uris()) {
        try
        {
            auto file = Gio::File::create_for_uri(uri);
            auto info = file->query_info(G_FILE_ATTRIBUTE_STANDARD_TYPE);

            if (!info || info->get_type() == Gio::FILE_TYPE_DIRECTORY)
                continue;

            auto source_app = Gio::DesktopAppInfo::create_from_filename(file->get_path());
            if (source_app)
            {
                auto app = app_manager->lookup_app(source_app->get_id());
                if (app)
                    KiranHelper::add_app_to_fixed_list(app);
                else {
                    std::string new_id;

                    new_id = app_manager->create_userapp_from_uri(uri);
                    LOG_WARNING("NEW user app created, id '%s'", new_id.c_str());
                    if (!new_id.empty()) {
                        Glib::signal_idle().connect_once(
                            [new_id]() -> void {
                                Kiran::TaskBarSkeleton::get_instance()->add_fixed_app(new_id);
                            });
                    }
                }
            }
            else
                LOG_WARNING("Failed to add fixed apps: can not create app for desktop file '%s'", uri.c_str());
        }
        catch (const Gio::Error &e)
        {
            LOG_WARNING("Failed to query information for '%s'", uri.c_str());
            continue;
        }
   }
}

void TasklistButtonsContainer::put_child_before(Gtk::Widget *source, Gtk::Widget *dest)
{
    int dest_pos;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        dest_pos = child_property_x(*dest).get_value();
        child_property_x(*source).set_value(dest_pos-1);
    } else {
        dest_pos = child_property_y(*dest).get_value();
        child_property_y(*source).set_value(dest_pos-1);
    }
    queue_allocate();
}

void TasklistButtonsContainer::put_child_after(Gtk::Widget *source, Gtk::Widget *dest)
{
    int dest_pos;

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        dest_pos = child_property_x(*dest).get_value();
        child_property_x(*source).set_value(dest_pos + 1);
    } else {
        dest_pos = child_property_y(*dest).get_value();
        child_property_y(*source).set_value(dest_pos + 1);
    }
    queue_allocate();
}

bool TasklistButtonsContainer::child_is_after(Gtk::Widget *w1, Gtk::Widget *w2)
{
    Gtk::Allocation a1, a2;

    a1 = w1->get_allocation();
    a2 = w2->get_allocation();

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return a1.get_x() > a2.get_x();
    else
        return a1.get_y() > a2.get_y();
}

bool TasklistButtonsContainer::child_is_before(Gtk::Widget *w1, Gtk::Widget *w2)
{
    Gtk::Allocation a1, a2;

    a1 = w1->get_allocation();
    a2 = w2->get_allocation();

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        return a1.get_x() < a2.get_x();
    else
        return a1.get_y() < a2.get_y();
}

void TasklistButtonsContainer::on_button_drag_motion(Gtk::Widget *source_widget, int x, int y)
{
    PointerMotionDirection motion_dir;      //拖动过程中的鼠标移动方向
    Gtk::Allocation allocation = source_widget->get_allocation();

    if (dragging_source == nullptr) {
        /* 记录被拖动的按钮对象，同时为其生成拖动过程中随鼠标移动的缩略图 */

        dragging_source = source_widget;
        dragging_icon = Gdk::Pixbuf::create(get_window(),
                                            allocation.get_x(),
                                            allocation.get_y(),
                                            allocation.get_width(),
                                            allocation.get_height());
    }


    /*
     * 根据上次记录的鼠标位置和当前鼠标位置，判断拖动方向
     */
    motion_dir = MOTION_DIR_UNKNOWN;
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
    {
        if (dragging_pos.get_x() != -1)
            motion_dir = (x > dragging_pos.get_x()) ? MOTION_DIR_RIGHT : MOTION_DIR_LEFT;
        dragging_pos.set_x(x);
        dragging_pos.set_y(0);
    }
    else
    {
        if (dragging_pos.get_x() != -1)
            motion_dir = (y > dragging_pos.get_y()) ? MOTION_DIR_UP : MOTION_DIR_DOWN;
        dragging_pos.set_x(0);
        dragging_pos.set_y(y);
    }

    /* 拖动过程中隐藏预览窗口 */
    previewer->hide();
    queue_draw();

    /* 重新计算被拖动的按钮排列位置 */
    reorder_child(source_widget, motion_dir);
}

void TasklistButtonsContainer::on_button_drag_end(Gtk::Widget *source_widget)
{
    dragging_source = nullptr;
    dragging_icon.clear();
    dragging_pos.set_x(-1);
    dragging_pos.set_y(-1);
    queue_draw();
}

void TasklistButtonsContainer::on_orientation_changed()
{
    Glib::RefPtr<Gtk::Adjustment> adjustment = get_adjustment();

    /*
     *  滚动方向发生变化后，adjustment对象也会发生变化.
     *  因此需要重新连接信号
     */
    init_paging_monitor();

    if (get_realized()) {
        /* 调整应用按钮大小 */
        for (auto button: get_children()) {
            button->queue_resize();
        }
    }
}

void TasklistButtonsContainer::init_paging_monitor()
{
    /*
     * 监控滚动区域变化(包括滚动区域本身和当前值)，大小变化和child变化都会导致滚动区域发生变化
     */
    auto adjustment = get_adjustment();

    if (adjustment_changed.connected())
        adjustment_changed.disconnect();
    adjustment_changed = adjustment->signal_changed().connect(
                [this]() -> void {
                    signal_page_changed().emit();
                    ensure_active_app_button_visible();
                });

    if (paging_notify.connected())
        paging_notify.disconnect();
    paging_notify = adjustment->signal_value_changed().connect(
        [this]() -> void {
            signal_page_changed().emit();

            /* 要重新定位面板上应用按钮对应的所有窗口最小化时的位置 */
            for (auto child: get_children()) {
                auto button = dynamic_cast<TasklistAppButton*>(child);
                Glib::signal_idle().connect_once(
                    sigc::mem_fun(*button, &TasklistAppButton::update_windows_icon_geometry));
            }
        });
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

    previewer->hide();

    for (auto child: get_children()) {
        auto button = dynamic_cast<TasklistAppButton*>(child);
        auto old_app = button->get_app();

        if (G_UNLIKELY(!old_app))
            continue;

        app_buttons.erase(old_app);
        if (old_app->get_kind() != Kiran::AppKind::FAKE_DESKTOP)
        {
            auto new_app = app_manager->lookup_app(old_app->get_desktop_id());
            if (new_app)
            {
                /* 对于未卸载应用，找到对应的应用，重新刷新窗口 */
                if (new_app->get_taskbar_windows().size() != 0 || KiranHelper::app_is_in_fixed_list(new_app))
                {
                    LOG_DEBUG("update button for app '%s'", old_app->get_desktop_id().c_str());
                    button->set_app(new_app);
                    app_buttons.insert(std::make_pair(new_app, button));
                    continue;
                } else {
                    /* 应用未卸载，但窗口已归属到其它的应用 */
                    LOG_DEBUG("remove button for app '%s', since no windows opened", old_app->get_desktop_id().c_str());
                }
            } else {
                /* 已卸载应用 */
                LOG_DEBUG("remove button for uninstalled app '%s'", old_app->get_desktop_id().c_str());
            }
        } else {
            /* Fake desktop 应用 */
            LOG_DEBUG("remove button for fake app '%s'", old_app->get_desktop_id().c_str());
        }

        /**
         * 对于已卸载应用和Fake Desktop应用，删除对应的应用按钮，
         * 原应用上的窗口随后会重新分配
         */
        remove(*button);
        delete button;
    }

    //加载常驻任务栏应用
    LOG_DEBUG("%s: loading fixed apps ...\n", __FUNCTION__);
    apps = backend->get_fixed_apps();
    for (auto app: apps) {
        add_app_button(app);
    }

    //加载当前运行应用
    LOG_DEBUG("%s: loading running apps ...\n", __FUNCTION__);
    if (backend->get_app_show_policy() == Kiran::TaskBarSkeleton::POLICY_SHOW_ALL)
    {
        apps = app_manager->get_running_apps();
        for (auto app: apps)
        {
            //如果应用的窗口都不需要在任务栏显示，就不在任务栏上显示应用按钮
            if (KiranHelper::get_taskbar_windows(app).size() == 0)
                continue;

            add_app_button(app);
        }
    } else {
        auto active_workspace = Kiran::WorkspaceManager::get_instance()->get_active_workspace();

        g_return_if_fail(active_workspace != nullptr);
        for (auto window: active_workspace->get_windows()) {
            on_window_opened(window);
        }
    }
}

/**
 * @brief KiranTasklistWidget::on_fixed_apps_added
 *        常驻任务栏应用增加时的回调函数，
 * @param apps
 */
void TasklistButtonsContainer::on_fixed_apps_added(const Kiran::AppVec &apps)
{
    LOG_DEBUG("Got fixed apps added signal\n");
    for (auto app: apps) {
        LOG_DEBUG("add fixed app '%s'", app->get_name().data());
        TasklistAppButton *button = find_app_button(app);
        if (!button) {
            //如果应用需要常驻任务栏，而且目前应用未打开，那就将其应用按钮添加到任务栏
           add_app_button(app);
        }
    }
}

void TasklistButtonsContainer::on_fixed_apps_removed(const Kiran::AppVec &apps)
{
    LOG_DEBUG("Got fixed apps removed signal\n");
    for (auto app: apps) {
        LOG_DEBUG("remove fixed app '%s', %d windows found\n",
                app->get_name().data(),
                (int)app->get_taskbar_windows().size());
        TasklistAppButton *button = find_app_button(app);

        if (button && KiranHelper::get_taskbar_windows(app).size() == 0) {
            //如果应用已经不再常驻任务栏，而且无已打开窗口，那就将其应用按钮从任务栏上移除
            remove_app_button(app);
        }
    }
}

void TasklistButtonsContainer::on_app_show_policy_changed()
{
    load_applications();
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
        LOG_WARNING("button not realized, allocation (%d, %d), %d x %d",
                  child_allocation.get_x(),
                  child_allocation.get_y(),
                  child_allocation.get_width(),
                  child_allocation.get_height());
        return;
    }

    view_size = static_cast<int>(adjustment->get_page_size());

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
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
    previewer->signal_leave_notify_event().connect_notify(
                sigc::hide(sigc::mem_fun(*this, &TasklistButtonsContainer::schedule_pointer_check)), true);
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

void TasklistButtonsContainer::get_child_geometry(Gtk::Widget *child, Gdk::Rectangle &rect)
{
    int child_x, child_y;
    Gtk::Allocation allocation = child->get_allocation();

    child->get_window()->get_origin(child_x, child_y);

    if (!child->get_has_window())
    {
        child_x += allocation.get_x();
        child_y += allocation.get_y();
    }

    rect.set_x(child_x);
    rect.set_y(child_y);
    rect.set_width(allocation.get_width());
    rect.set_height(allocation.get_height());
}

void TasklistButtonsContainer::get_pointer_position(int &pointer_x, int &pointer_y)
{
    auto pointer = get_display()->get_default_seat()->get_pointer();
    pointer->get_position(pointer_x, pointer_y);
}

void TasklistButtonsContainer::reorder_child(Gtk::Widget *widget, PointerMotionDirection motion_dir)
{
    bool found = false;
    Gtk::Orientation orient = get_orientation();

    if (motion_dir != MOTION_DIR_UNKNOWN)
    {
        int pointer_x, pointer_y;
        std::vector<Gtk::Widget *> children = get_children();

        /* 按照屏幕位置将按钮从左到右(或从上向下)进行排序 */
        std::sort(children.begin(), children.end(),
                  [this](Gtk::Widget *c1, Gtk::Widget *c2) -> bool {
                        return this->child_is_before(c1, c2);
                  });

        /* 获取当前鼠标位置 */
        get_pointer_position(pointer_x, pointer_y);

        for (auto child : children)
        {
            Gdk::Rectangle rect;

            if (!child->is_visible())
                continue;

            get_child_geometry(child, rect);

            GdkPoint point;

            point.x = pointer_x;
            point.y = pointer_y;

            if (KiranHelper::gdk_rectangle_contains_point(rect.gobj(), &point))
            {
                /*
                 * 鼠标位于按钮上方
                 */
                if (child != widget) {

                    switch (motion_dir) {
                    case MOTION_DIR_LEFT:
                    case MOTION_DIR_UP:
                        if (child_is_before(child, widget))
                            put_child_after(child, widget);
                        break;
                    case MOTION_DIR_RIGHT:
                    case MOTION_DIR_DOWN:
                        if (child_is_after(child, widget))
                            put_child_before(child, widget);
                        break;
                    default:
                        LOG_CRITICAL("shouldn't get here");
                        break;
                    }
                }
                found = true;
                break;
            } else if ((orient == Gtk::ORIENTATION_HORIZONTAL && pointer_x < rect.get_x()) ||
                       (orient== Gtk::ORIENTATION_VERTICAL && pointer_y < rect.get_y()))
            {
                /*
                 * 鼠标位于按钮之间的间隙
                 */
                 if (widget != child)
                    put_child_before(widget, child);

                found = true;
                break;
            }
        }

        if (!found) {
            /* 鼠标位于所有按钮的最右侧或最下方 */
            if (children.size() > 0)
                put_child_after(widget, children.back());
            else
                put_child_after(widget, nullptr);
        }
    }
}

void TasklistButtonsContainer::init_dnd()
{
    std::vector<Gtk::TargetEntry> targets;

    targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TARGET_OTHER_APP));
    drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
}