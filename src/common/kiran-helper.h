/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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

    static bool window_is_on_active_workspace(const std::shared_ptr<Kiran::Window> &window);
    static Kiran::WindowVec get_active_workspace_windows(const std::shared_ptr<Kiran::App> &app);

    static Kiran::WindowVec get_taskbar_windows(const std::shared_ptr<Kiran::App> &app);
};

void show_applet_about_dialog(const char *program_name,
                              const char *icon_name,
                              const char *comments);

#endif  // KIRANHELPER_H
