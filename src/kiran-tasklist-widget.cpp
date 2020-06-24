#include "kiran-tasklist-widget.h"
#include <window-manager.h>
#include <window.h>

KiranTasklistWidget::KiranTasklistWidget(MatePanelApplet *applet_):
    applet(applet_)
{
    manager = Kiran::AppManager::get_instance();
    auto apps = manager->get_running_apps();

    set_column_spacing(10);
    update_orientation();

    for (auto app: apps) {
        if (app->get_taskbar_windows().size() == 0)
            continue;

        add_app_button(app);
    }

    previewer = new KiranAppPreviewer();
    active_app = get_current_active_app();

    auto window_manager = Kiran::WindowManager::get_instance();
    window_manager->signal_active_window_changed().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_active_window_changed));
    window_manager->signal_window_opened().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_window_opened));
    window_manager->signal_window_closed().connect(
                sigc::mem_fun(*this, &KiranTasklistWidget::on_window_closed));


    auto context = get_style_context();
    context->add_class("tasklist-widget");
}

KiranTasklistWidget::~KiranTasklistWidget()
{
    delete previewer;
}

void KiranTasklistWidget::add_app_button(const KiranAppPointer &app)
{
    if (find_app_button(app))
        return;

    auto button = Gtk::manage(new KiranTasklistAppButton(app));
    button->set_orientation(get_orientation());
    button->show_all();

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
    add(*button);

    app_buttons.insert(std::make_pair(app, button));
}

void KiranTasklistWidget::remove_app_button(const KiranAppPointer &app)
{
    auto button = find_app_button(app);
    if (!button)
        return;

    remove(*button);
    app_buttons.erase(app);
}

KiranAppPointer KiranTasklistWidget::get_current_active_app()
{
    auto window_manager = Kiran::WindowManager::get_instance();
    return window_manager->get_active_window()->get_app();
}

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

        g_message("active app changed, new '%s', old '%s'\n",
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
    } else
        g_message("active app not change\n");
}

void KiranTasklistWidget::on_window_opened(KiranWindowPointer window)
{
    g_message("window '%s' opened\n", window->get_name().data());
    auto app = window->get_app();
    if (!app) {
        g_warning("no app found for new window '%s'\n", window->get_name().data());
        return;
    }

    auto app_button = find_app_button(app);
    if (!app_button) {
        g_message("create new button for app '%s'\n", app->get_name().data());
        add_app_button(app);
    }
    else {
        g_message("app button already exists\n");
        /**
         * 检查app的预览窗口是否打开，如果打开，需要更新预览窗口内容
         */
        if (previewer->is_visible() && previewer->get_app() == app) {
            previewer->add_window_previewer(window, true);
        }
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
            g_message("remove app button '%s'\n", app->get_name().data());
            remove_app_button(app);
        }
    }
}

void KiranTasklistWidget::move_previewer(KiranTasklistAppButton *target_button)
{
    Gtk::PositionType pos = Gtk::POS_BOTTOM;

    previewer->set_idle(false);
    previewer->set_app(target_button->get_app());

    if (applet) {
        switch (mate_panel_applet_get_orient(applet))
        {
        case MATE_PANEL_APPLET_ORIENT_DOWN:
            pos = Gtk::POS_TOP;
            break;
        case MATE_PANEL_APPLET_ORIENT_UP:
            pos = Gtk::POS_BOTTOM;
            break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
            pos = Gtk::POS_RIGHT;
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            pos = Gtk::POS_LEFT;
            break;
        }
    }

    Glib::signal_idle().connect([this, target_button, pos]() -> bool {
        this->previewer->set_relative_to(target_button, pos);
        this->previewer->show();
        return false;
    });
}

void KiranTasklistWidget::hide_previewer()
{
    previewer->set_idle(true);

    Glib::signal_idle().connect(
                [this]() -> bool {
                    if (this->previewer->get_idle()) {
                        this->previewer->hide();
                    }
                    return false;
                });
}

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
