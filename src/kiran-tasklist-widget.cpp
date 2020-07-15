#include "kiran-tasklist-widget.h"
#include <window-manager.h>
#include <window.h>
#include "taskbar-skeleton.h"

#define PREVIEWER_ANIMATION_TIMEOUT 400

void on_applet_size_change(MatePanelApplet *applet, gint size, gpointer userdata)
{
    Gtk::Container *widget = static_cast<Gtk::Container*>(userdata);

    g_debug("applet size changed, resize now\n");
    widget->queue_resize();
}

void on_applet_size_allocate(MatePanelApplet *applet, GdkRectangle *rect, gpointer userdata)
{
    Gtk::Requisition min, natural;
    Gtk::Allocation allocation;
    Gtk::Container *widget = static_cast<Gtk::Container*>(userdata);
    widget->get_preferred_size(min, natural);
    allocation = widget->get_allocation();
    g_debug("applet size allocate, natural %d, allocated %d\n",
            natural.width,
            allocation.get_width());

    int *hints = (int*)malloc(2 * sizeof(int));
    hints[0] = allocation.get_width();
    hints[1] = allocation.get_height();
    mate_panel_applet_set_size_hints(applet, hints, 2, 0);
    free(hints);
}

KiranTasklistWidget::KiranTasklistWidget(MatePanelApplet *applet_):
    applet(applet_)
{
    manager = Kiran::AppManager::get_instance();

    set_spacing(6);
    update_orientation();

    previewer = new KiranAppPreviewer();
    active_app = get_current_active_app();

    //响应窗口变化信号
    auto window_manager = Kiran::WindowManager::get_instance();
    window_manager->signal_active_window_changed().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_active_window_changed));
    window_manager->signal_window_opened().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_window_opened));
    window_manager->signal_window_closed().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_window_closed));

    //响应常住任务栏应用变化信号
    auto backend = Kiran::TaskBarSkeleton::get_instance();
    backend->signal_fixed_app_added().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_fixed_apps_added));
    backend->signal_fixed_app_deleted().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_fixed_apps_removed));

    g_signal_connect(applet, "change-size", G_CALLBACK(on_applet_size_change), this);
    g_signal_connect(applet, "size-allocate", G_CALLBACK(on_applet_size_allocate), this);

    //响应预览窗口打开信号
    previewer->signal_opened().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_previewer_window_opened));

    //Schedule: 加载应用按钮
    Glib::signal_idle().connect_once(
                sigc::mem_fun(*this, &KiranTasklistWidget::load_applications));

    get_style_context()->add_class("tasklist-widget");
}

KiranTasklistWidget::~KiranTasklistWidget()
{
    delete previewer;
}

/**
 * @brief KiranTasklistWidget::add_app_button
 *        为给定的应用创建应用按钮，并添加到任务栏. 如果应用对应的按钮在任务栏上已经存在，就直接返回
 * @param app 待添加的应用
 */
void KiranTasklistWidget::add_app_button(const KiranAppPointer &app)
{
    if (!app || find_app_button(app))
        return;

    auto button = Gtk::manage(new KiranTasklistAppButton(app));
    button->set_orientation(get_orientation());
    button->show_all();
#if 1
    //鼠标进入应用按钮时，显示预览窗口
    button->signal_enter_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(
                        sigc::bind<KiranTasklistAppButton*>(
                            sigc::mem_fun(*this, &KiranTasklistWidget::move_previewer),
                            button)),
                    false));


    //鼠标离开应用按钮时，隐藏预览窗口
    button->signal_leave_notify_event().connect(
                sigc::bind_return<bool>(
                    sigc::hide(sigc::mem_fun(*this, &KiranTasklistWidget::hide_previewer)),
                    false));
#endif
    add(*button);
    button->show_all();

    //建立app和应用按钮之间的映射
    app_buttons.insert(std::make_pair(app, button));
    g_debug("Add app button '%s'(%p)\n", app->get_name().data(), app.get());

}

/**
 * @brief KiranTasklistWidget::remove_app_button 从任务栏上删除给定应用对应的应用按钮，如果应用按钮未找到，就直接返回
 * @param app 待删除的应用
 *
 */
void KiranTasklistWidget::remove_app_button(const KiranAppPointer &app)
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
KiranAppPointer KiranTasklistWidget::get_current_active_app()
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
KiranTasklistAppButton *KiranTasklistWidget::find_app_button(
        const KiranAppPointer &app)
{
    if (!app)
        return nullptr;

    auto iter = app_buttons.find(app);
    if (iter == app_buttons.end())
        return nullptr;

    return iter->second;
}

void KiranTasklistWidget::on_active_window_changed(KiranWindowPointer previous,
                                                   KiranWindowPointer active)
{
    KiranAppPointer current_app;

    if (KiranHelper::window_is_ignored(active))
        current_app = nullptr;
    else
        current_app = active->get_app();

    if (current_app != active_app) {
        //active app changed
        KiranTasklistAppButton *last_button, *active_button;

        g_debug("active app changed, new '%s', old '%s'\n",
                  current_app?current_app->get_name().data():"null",
                  active_app?active_app->get_name().data():"null");

        last_button = find_app_button(active_app);
        active_button = find_app_button(current_app);

        if (last_button) {
            last_button->refresh();
        }

        if (active_button) {
            active_button->refresh();
        }


        active_app = current_app;
    }
}

void KiranTasklistWidget::on_window_opened(KiranWindowPointer window)
{
    g_debug("window '%s' opened\n", window->get_name().data());
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
        /**
         * 检查app的预览窗口是否打开，如果打开，需要更新预览窗口内容
         */
        if (previewer->is_visible() && previewer->get_app() == app) {
            previewer->add_window_previewer(window, true);
        } else
            app_button->refresh();
    }
}

void KiranTasklistWidget::on_window_closed(KiranWindowPointer window)
{
    auto app = window->get_app();
    if (!app)
        return;

    if (previewer->is_visible() && previewer->get_app() == app) {
        //从预览窗口中删除该窗口的预览图
        previewer->remove_window_previewer(window);
    }

    auto app_button = find_app_button(app);
    if (app_button) {
        /**
         * 检查对应的应用是否有剩余窗口，如果没有，就删除该应用按钮
         */
        if (app->get_taskbar_windows().size() == 0) {
            //TODO 需要检查是否属于常驻任务栏的应用
            if (!KiranHelper::app_is_in_fixed_list(app))
                remove_app_button(app);
            else
                app_button->refresh();
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
void KiranTasklistWidget::move_previewer(KiranTasklistAppButton *target_button)
{
    Gtk::PositionType pos = Gtk::POS_BOTTOM;

    if (target_button->get_app()->get_taskbar_windows().size() == 0) {
        return;
    }

    previewer->set_idle(false);
    previewer->set_app(target_button->get_app());

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
            pos = Gtk::POS_RIGHT;
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            pos = Gtk::POS_LEFT;
            break;
        }
    }

    Glib::signal_timeout().connect_once([this, target_button, pos]() -> void {
        if (this->previewer->get_idle() || target_button->get_context_menu_opened()) {
            //如果目标应用按钮的右键菜单已经打开，就没有必要再显示预览窗口
            g_message("previewer idle or button menu opened\n");
            return;
        }
        this->previewer->set_relative_to(target_button, pos);
        this->previewer->show();
    }, PREVIEWER_ANIMATION_TIMEOUT);
}

void KiranTasklistWidget::hide_previewer()
{
    previewer->set_idle(true);

    Glib::signal_timeout().connect_once(
                [this]() -> void {
                    if (this->previewer->get_idle()) {
                        this->previewer->hide();
                    }
                }, PREVIEWER_ANIMATION_TIMEOUT);
}

/**
 * @brief KiranTasklistWidget::update_orientation
 *        根据applet的排列方向设置任务栏应用按钮的排列方向
 */
void KiranTasklistWidget::update_orientation()
{
    if (!applet)
        return;

    switch (mate_panel_applet_get_orient(applet))
    {
    case MATE_PANEL_APPLET_ORIENT_DOWN:
    case MATE_PANEL_APPLET_ORIENT_UP:
        set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
        set_orientation(Gtk::ORIENTATION_VERTICAL);
        break;
    }
}

void KiranTasklistWidget::get_preferred_width_vfunc(int &min_width, int &natural_width) const
{
    unsigned int applet_size = mate_panel_applet_get_size(applet);

    natural_width = 0;
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        int min, natural;
        for (auto child: get_children()) {
            child->get_preferred_width(min, natural);
            natural_width += natural;
        }
        min_width = natural_width;
    } else {
        min_width = natural_width = applet_size;
    }
}

void KiranTasklistWidget::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{
    unsigned int applet_size = mate_panel_applet_get_size(applet);

    min_height = natural_height = 0;
    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL) {
        min_height = natural_height = applet_size;
    } else {
        int min, natural;
        for (auto child: get_children()) {
            child->get_preferred_height(min, natural);
            natural_height += natural;
        }
    }
}
void KiranTasklistWidget::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::Requisition min, natural;

    get_preferred_size(min, natural);

    if (get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        allocation.set_width(natural.width);
    else
        allocation.set_height(natural.height);
    g_debug("KiranTasklistWidget old allocation %d x %d, natural %d x %d\n",
            allocation.get_width(),
            allocation.get_height(),
            natural.width,
            natural.height);

    Gtk::Box::on_size_allocate(allocation);;
}

/**
 * @brief KiranTasklistWidget::load_applications
 *        加载并显示应用按钮，具体包括常驻任务栏应用和已打开应用
 */
void KiranTasklistWidget::load_applications()
{
    Kiran::AppVec apps;
    Kiran::TaskBarSkeleton *backend = Kiran::TaskBarSkeleton::get_instance();

    //加载常驻任务栏应用
    g_debug("%s: loading fixed apps ...\n", __FUNCTION__);
    apps = backend->get_fixed_apps();
    for (auto app: apps) {
        add_app_button(app);
    }

    //加载当前运行应用
    g_debug("%s: loading running apps ...\n", __FUNCTION__);
    apps = manager->get_running_apps();
    for (auto app: apps) {
        if (find_app_button(app))
            continue;

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
void KiranTasklistWidget::on_fixed_apps_added(const Kiran::AppVec &apps)
{
    g_debug("Got fixed apps added signal\n");
    for (auto app: apps) {
        g_debug("%s: add fixed app '%s'\n", __func__,
                app->get_name().data());
        KiranTasklistAppButton *button = find_app_button(app);
        if (!button) {
            //如果应用需要常驻任务栏，而且目前应用未打开，那就将其应用按钮添加到任务栏
           add_app_button(app);
        }
    }
}

void KiranTasklistWidget::on_fixed_apps_removed(const Kiran::AppVec &apps)
{
    g_debug("Got fixed apps removed signal\n");
    for (auto app: apps) {
        g_debug("%s: remove fixed app '%s', %d windows found\n",
                __func__,
                app->get_name().data(),
                app->get_taskbar_windows().size());
        KiranTasklistAppButton *button = find_app_button(app);

        if (button && app->get_taskbar_windows().size() == 0) {
            //如果应用已经不再常驻任务栏，而且无已打开窗口，那就将其应用按钮从任务栏上移除
            remove_app_button(app);
        }
    }
}

void KiranTasklistWidget::on_previewer_window_opened()
{
    for (auto data: app_buttons) {
        auto button = data.second;
        button->on_previewer_opened();
    }
}
