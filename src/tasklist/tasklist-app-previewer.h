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

#ifndef TASKLIST_APP_PREVIEWER_H
#define TASKLIST_APP_PREVIEWER_H

#include <gtkmm.h>
#include "tasklist-window-previewer.h"
//
#include "tasklist-app-button.h"
//
#include <X11/Xlib.h>
#include "app.h"

#define DEBUG

class TasklistAppPreviewer : public Gtk::Window
{
public:
    TasklistAppPreviewer();

    /**
     * @brief set_app 设置预览窗口关联的app应用，关联后会重新加载窗口缩略图列表
     * @param app     要关联的应用
     */
    void set_app(const std::shared_ptr<Kiran::App> &app);

    /**
     * @brief get_app 获取预览窗口当前关联的应用
     * @return      返回预览窗口关联的应用
     */
    const std::shared_ptr<Kiran::App> get_app() const;

    /**
     * @brief set_relative_to 根据给定的目标应用按钮和显示位置调整预览窗口的显示位置
     * @param button    目标应用按钮
     * @param pos       预览窗口相对于button的显示位置
     */
    void set_relative_to(TasklistAppButton *button, Gtk::PositionType pos);

    /**
     * @brief add_window_thumbnail  将给定窗口的缩略图添加到预览窗口中显示
     * @param window    待添加的窗口
     */
    void add_window_thumbnail(std::shared_ptr<Kiran::Window> &window);

    /**
     * @brief remove_window_thumbnail 将给定窗口的缩略图从预览窗口中移除
     * @param window    待移除的窗口
     */
    void remove_window_thumbnail(std::shared_ptr<Kiran::Window> &window);

    /**
     * @brief get_thumbnails_count  获取预览窗口中窗口缩略图的个数
     * @return  返回窗口中缩略图的个数
     */
    unsigned long get_thumbnails_count();

    /**
     * @brief has_context_menu_opened  检查预览窗口中是否有缩略图的右键菜单打开
     * @return  有菜单打开返回true，否则返回false
     */
    bool has_context_menu_opened();

    /**
     * @brief contains_pointer         检查当前鼠标是否位于预览窗口中
     * @return  在预览窗口中返回true，否则返回false
     */
    bool contains_pointer() const;

    /**
     * @brief get_scrollbar            根据缩略图排列方向获取窗口内的对应滚动条
     * @return  返回获取到的滚动条
     */
    const Gtk::Scrollbar *get_scrollbar() const;
    Gtk::Scrollbar *get_scrollbar();

protected:
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const override;
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_scroll_event(GdkEventScroll *event) override;

    /**
     * @brief on_child_context_menu_toggled 回调函数，窗口缩略图右键菜单切换显示时调用
     * @param active    缩略图右键菜单是否打开
     */
    virtual void on_child_context_menu_toggled(bool active);

    /**
     * @brief init_ui      初始化界面
     */
    virtual void init_ui();

    /**
     * @brief adjust_size  调整预览窗口尺寸到最佳大小
     */
    virtual void adjust_size();

    /**
     * @brief reposition   调整预览窗口显示位置
     */
    void reposition();

    /**
     * @brief set_position 设置预览窗口相对于所属应用按钮的显示位置
     * @param pos          相对显示位置
     */
    void set_position(Gtk::PositionType pos);

private:
    Gtk::ScrolledWindow scroll_window;
    Gtk::Box box;

    TasklistAppButton *relative_to; /* 所属的应用按钮 */
    Gtk::PositionType position;     /* 相对于所属的应用按钮的显示位置 */

    std::weak_ptr<Kiran::App> app;                                     /* 所属的应用 */
    std::map<unsigned long, TasklistWindowPreviewer *> win_previewers; /* 窗口XID和缩略图之间的映射表 */

    /**
     * @brief load_windows_list 加载所属应用的已打开窗口缩略图
     */
    void load_windows_list();

    /**
     * @brief set_rgba_visual 设置窗口使用RGBA Visual，支持Alpha透明度
     */
    void set_rgba_visual();

    static const int border_spacing;
};

#endif  // TASKLIST_APP_PREVIEWER_H
