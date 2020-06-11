/*
 * @Author       : tangjie02
 * @Date         : 2020-06-11 09:30:42
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 17:19:24
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-display.cpp
 */

#include <gtkmm.h>

#include <cinttypes>

#include "lib/core_worker.h"
#include "test/test-menu-common.h"

void sig_hander(int signo)
{
}

void window_opened(std::shared_ptr<Kiran::Window> window)
{
    g_print("signal: window '%s' is opened\n", window->get_name().c_str());
}

void window_closed(std::shared_ptr<Kiran::Window> window)
{
    g_print("signal: window '%s' is closed\n", window->get_name().c_str());
}

void app_opened(std::shared_ptr<Kiran::App> app)
{
    g_print("signal: app '%s' is opened\n", app->get_desktop_id().c_str());
}

void app_closed(std::shared_ptr<Kiran::App> app)
{
    g_print("signal: app '%s' is closed\n", app->get_desktop_id().c_str());
}

void window_change_for_app(std::shared_ptr<Kiran::App> app)
{
    g_print("signal: the window of the app '%s' is changed\n", app->get_desktop_id().c_str());
}

void workspace_created(std::shared_ptr<Kiran::Workspace> workspace)
{
    g_print("signal: workspace '%s' is created\n", workspace->get_name().c_str());
}

void workspace_destroyed(std::shared_ptr<Kiran::Workspace> workspace)
{
    g_print("signal: workspace '%s' is destroyed\n", workspace->get_name().c_str());
}

gboolean timing_print(gpointer user_data)
{
    /*-------------------------------------------  relate to window --------------------------------------*/
    {
        g_print("--------------------------- relate to window ---------------------------\n\n");

        auto window_manager = Kiran::WindowManager::get_instance();

        window_manager->signal_window_opened().connect(&window_opened);
        window_manager->signal_window_closed().connect(&window_closed);

        auto windows = window_manager->get_windows();

        g_print("total window number: %d\n", windows.size());
        for (int i = 0; i < windows.size(); ++i)
        {
            auto window = windows[i];
            auto app = window->get_app();

            g_print("window_name: %s xid: %" PRIu64 "\n",
                    window->get_name().c_str(),
                    window->get_xid());

            g_print("   icon_name:              %s\n", window->get_icon_name().c_str());
            g_print("   class_group_name:       %s\n", window->get_class_group_name().c_str());
            g_print("   class_instance_name:    %s\n", window->get_class_instance_name().c_str());
            g_print("   pid:                    %d\n", window->get_pid());
            g_print("   window_type:            %d\n", window->get_window_type());
            g_print("   is_active:              %d\n", window->is_active());
            g_print("   app_id:                 %s\n", app ? app->get_desktop_id().c_str() : "null");

            g_print("\n\n");
        }

        if (windows.size() > 0)
        {
            auto window = windows.back();
            // window->activate();
            // window->minimize();
            // window->maximize();
            // window->unmaximize();
            // window->make_above();
            // window->close();
        }
    }

    /*-------------------------------------------  relate to app --------------------------------------*/
    {
        g_print("--------------------------- relate to app ---------------------------\n\n");

        auto app_manager = Kiran::AppManager::get_instance();

        app_manager->signal_app_opened().connect(&app_opened);
        app_manager->signal_app_closed().connect(&app_closed);
        app_manager->signal_window_change_for_app().connect(&window_change_for_app);

        auto apps = app_manager->get_running_apps();

        g_print("total running app number: %d\n", apps.size());

        for (int i = 0; i < apps.size(); ++i)
        {
            auto app = apps[i];
            g_print("running app_id: %s\n", app->get_desktop_id().c_str());

            auto windows = app->get_windows();
            for (int j = 0; j < windows.size(); ++j)
            {
                auto window = windows[j];
                g_print("   related window name: %s\n", window->get_name().c_str());
            }
            g_print("\n\n");
        }

        if (apps.size() > 0)
        {
            auto app = apps.back();
            // app->close_all_windows();
        }
    }

    /*-------------------------------------------  relate to workspace --------------------------------------*/

    {
        g_print("--------------------------- relate to workspace ---------------------------\n\n");

        auto workspace_manager = Kiran::WorkspaceManager::get_instance();

        workspace_manager->signal_workspace_created().connect(&workspace_created);
        workspace_manager->signal_workspace_destroyed().connect(&workspace_destroyed);

        workspace_manager->change_workspace_count(2);
        auto workspaces = workspace_manager->get_workspaces();

        g_print("total workspace number: %d\n", workspaces.size());

        for (int i = 0; i < workspaces.size(); ++i)
        {
            auto workspace = workspaces[i];
            g_print("workspace_name: %s\n", workspace->get_name().c_str());

            auto windows = workspace->get_windows();
            for (int j = 0; j < windows.size(); ++j)
            {
                auto window = windows[j];
                g_print("   window_name: %s xid: %" PRIu64 " is_pinned: %d\n",
                        window->get_name().c_str(),
                        window->get_xid(),
                        window->is_pinned());
            }
            g_print("\n\n");
        }
    }

    return TRUE;
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    signal(SIGTRAP, sig_hander);

    Gtk::Main kit(argc, argv);

    Kiran::init_backend_system();

    timing_print(NULL);

    g_timeout_add_seconds(5, timing_print, NULL);

    kit.run();

    return 0;
}