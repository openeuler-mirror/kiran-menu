/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#ifndef KIRANHELPER_H
#define KIRANHELPER_H

#include <app-manager.h>
#include <gtkmm.h>
#include "global.h"

using KiranAppPointer = std::shared_ptr<Kiran::App>;
using KiranWindowPointer = std::shared_ptr<Kiran::Window>;
using KiranWorkspacePointer = std::shared_ptr<Kiran::Workspace>;

#define WINDOW_WIDTH(w) (std::get<2>(w->get_client_window_geometry()))
#define WINDOW_HEIGHT(w) (std::get<3>(w->get_client_window_geometry()))

#define UNUSED __attribute__((unused))

class KiranHelper
{
public:
    KiranHelper() = delete;
    ~KiranHelper() = delete;

    static void remove_widget(Gtk::Widget &widget);
    static void remove_all_for_container(Gtk::Container &container, bool need_free = true);
    static bool grab_input(Gtk::Widget &widget);
    static void ungrab_input(Gtk::Widget &widget);

    static bool window_is_ignored(KiranWindowPointer window);

    static bool app_is_in_favorite(const std::shared_ptr<Kiran::App> &app);
    static bool add_app_to_favorite(const std::shared_ptr<Kiran::App> &app);
    static bool remove_app_from_favorite(const std::shared_ptr<Kiran::App> &app);

    static bool app_is_in_fixed_list(const std::shared_ptr<Kiran::App> &app);
    static bool add_app_to_fixed_list(const std::shared_ptr<Kiran::App> &app);
    static bool remove_app_from_fixed_list(const std::shared_ptr<Kiran::App> &app);

    static bool gdk_rectangle_contains_point(GdkRectangle *rect, GdkPoint *point);
    static void cairo_draw_rounded_rect(const Cairo::RefPtr<Cairo::Context> &cr, Gdk::Rectangle &rect, double radius);
    static void geometry_to_rect(const Kiran::WindowGeometry &geometry, Gdk::Rectangle &rect);

    static void run_commandline(const char *cmdline);

    /**
     * @brief 从指定的app列表中启动第一个当前系统中存在的app
     * @param app_names   指定的app名称列表，最后一个元素需要以nullptr(NULL)结尾
     * @return  启动成功返回true，失败返回false
     */
    static bool launch_app_from_list(const char **app_names);

    static bool window_is_on_active_workspace(const std::shared_ptr<Kiran::Window> &window);
    static Kiran::WindowVec get_active_workspace_windows(const std::shared_ptr<Kiran::App> &app);

    static Kiran::WindowVec get_taskbar_windows(const std::shared_ptr<Kiran::App> &app);
};

void show_applet_about_dialog(const char *program_name,
                              const char *icon_name,
                              const char *comments);

#endif  // KIRANHELPER_H
