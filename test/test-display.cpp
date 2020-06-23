/*
 * @Author       : tangjie02
 * @Date         : 2020-06-11 09:30:42
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-23 17:34:40
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/test/test-display.cpp
 */

#include <gdk/gdk.h>
#include <gtkmm.h>
// xlib.h must be defined after gdkmm header file.
#include <cairomm/xlib_surface.h>
#include <gdk/gdkx.h>

#include <cinttypes>

#include "lib/core_worker.h"
#include "test/test-menu-common.h"

void sig_hander(int signo)
{
}

void window_opened(std::shared_ptr<Kiran::Window> window)
{
    g_print("signal: window '%s' is opened. xid: %" PRIu64 "\n",
            window->get_name().c_str(),
            window->get_xid());
}

void window_closed(std::shared_ptr<Kiran::Window> window)
{
    g_print("signal: window '%s' is closed. xid: %" PRIu64 "\n",
            window->get_name().c_str(),
            window->get_xid());
}

void active_window_changed(std::shared_ptr<Kiran::Window> prev_active_window, std::shared_ptr<Kiran::Window> cur_active_window)
{
    g_print("signal: active window change. prev: '%s' prev_xid: %" PRIu64 " cur: '%s' cur_xid: %" PRIu64 "\n",
            prev_active_window ? prev_active_window->get_name().c_str() : "null",
            prev_active_window ? prev_active_window->get_xid() : 0,
            cur_active_window ? cur_active_window->get_name().c_str() : "null",
            cur_active_window ? cur_active_window->get_xid() : 0);
}

void app_action_changed(std::shared_ptr<Kiran::App> app, Kiran::AppAction action)
{
    switch (action)
    {
        case Kiran::AppAction::APP_OPENED:
            g_print("signal: app '%s' is opened.\n", app->get_desktop_id().c_str());
            // g_print("signal: app '%s' is opened. xid: %" PRIu64 " pid: %" PRIu64 "\n",
            //         app->get_desktop_id().c_str(),
            //         app->get_xid(),
            //         app->get_pid());
            break;
        case Kiran::AppAction::APP_CLOSED:
            g_print("signal: app '%s' is closed.\n", app->get_desktop_id().c_str());

            // g_print("signal: app '%s' is closed. xid: %" PRIu64 " pid: %" PRIu64 "\n",
            //         app->get_desktop_id().c_str(),
            //         app->get_xid(),
            //         app->get_pid());
            break;
        case Kiran::AppAction::APP_WINDOW_CHANGED:
            g_print("signal: the window of the app '%s' is changed\n", app->get_desktop_id().c_str());
            break;
        case Kiran::AppAction::APP_ALL_WINDOWS_CLOSED:
            g_print("signal: the all windows of the app '%s' is closed\n", app->get_desktop_id().c_str());
    }
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
            g_print("   is_pinned:              %d\n", window->is_pinned());
            g_print("   is_above:               %d\n", window->is_above());
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

            auto taskbar_windows = app->get_taskbar_windows();
            for (int j = 0; j < taskbar_windows.size(); ++j)
            {
                auto window = windows[j];
                g_print("   related tarskbar window name: %s\n", window->get_name().c_str());
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

    /*------------------------------------------- save the screenshot of all windows --------------------------------------*/

    {
        g_print("---------------------------save the screenshot of all windows ---------------------------\n\n");

        auto window_manager = Kiran::WindowManager::get_instance();
        auto windows = window_manager->get_windows();
        auto display = gdk_x11_get_default_xdisplay();
        for (int i = 0; i < windows.size(); ++i)
        {
            auto window = windows[i];
            auto pixmap = window->get_pixmap();
            auto geometry = window->get_geometry();
            auto name = window->get_name();
            auto xid = window->get_xid();
            std::ostringstream oss;
            auto new_name = name;
            std::replace(new_name.begin(), new_name.end(), ' ', '-');
            std::replace(new_name.begin(), new_name.end(), ':', '-');
            oss << new_name << "_" << xid << ".png";

            g_print("save screentshot: name: %s xid: %" PRIu64 " pixmap: %" PRIu64 " width: %d height: %d\n",
                    name.c_str(),
                    xid,
                    pixmap,
                    std::get<2>(geometry),
                    std::get<3>(geometry));

            if (pixmap)
            {
                XWindowAttributes attrs;
                XGetWindowAttributes(display, xid, &attrs);
                auto surface = Cairo::XlibSurface::create(display, pixmap, attrs.visual, attrs.width, attrs.height);
                surface->write_to_png(oss.str());
            }
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

    auto window_manager = Kiran::WindowManager::get_instance();

    window_manager->signal_window_opened().connect(&window_opened);
    window_manager->signal_window_closed().connect(&window_closed);
    window_manager->signal_active_window_changed().connect(&active_window_changed);

    auto app_manager = Kiran::AppManager::get_instance();

    app_manager->signal_app_action_changed().connect(&app_action_changed);

    auto workspace_manager = Kiran::WorkspaceManager::get_instance();

    workspace_manager->signal_workspace_created().connect(&workspace_created);
    workspace_manager->signal_workspace_destroyed().connect(&workspace_destroyed);

    // Kiran::ScreenManager::global_init();

    timing_print(NULL);

    // g_timeout_add_seconds(10, timing_print, NULL);

    kit.run();

    return 0;
}