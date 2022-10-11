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

#ifndef TASKLIST_BUTTONS_CONTAINER_H
#define TASKLIST_BUTTONS_CONTAINER_H

#include <mate-panel-applet.h>
#include <map>
#include <vector>
#include "app-manager.h"
#include "app.h"
#include "kiran-helper.h"
#include "tasklist-app-button.h"
#include "tasklist-app-previewer.h"

typedef enum
{
    MOTION_DIR_UNKNOWN,
    MOTION_DIR_LEFT,  /* 往左拖动 */
    MOTION_DIR_RIGHT, /* 往右拖动 */
    MOTION_DIR_UP,    /* 往上拖动 */
    MOTION_DIR_DOWN   /* 往右拖动 */
} PointerMotionDirection;

class TasklistButtonsContainer : public Gtk::Layout
{
public:
    TasklistButtonsContainer(MatePanelApplet *applet_, int child_spacing = 15);
    ~TasklistButtonsContainer() override;

    Glib::PropertyProxy<Gtk::Orientation> property_orient();

    /**
     * @brief   get_current_active_app 获取当前活动窗口对应的应用
     * @return  返回活动窗口对应的应用，如果没有当前活动窗口，返回空应用
     */
    static KiranAppPointer get_current_active_app();

    /**
     * @brief update_orientation 根据applet所在面板的排列方向更新应用按钮排列方向
     */
    void update_orientation();

    /**
     * @brief get_orientation  获取应用按钮的排列方向
     * @return 返回应用按钮的排列方向
     */
    Gtk::Orientation get_orientation() const;

    /**
     * @brief on_applet_size_change  回调函数：所属的Applet插件尺寸变化时调用
     */
    void on_applet_size_change();

    /**
     * @brief get_applet_size       获取所属插件的尺寸
     * @return  返回插件尺寸
     */
    int get_applet_size() const;

    /**
     * @brief move_to_next_page     移动并显示下一页的应用按钮
     */
    void move_to_next_page();

    /**
     * @brief move_to_previous_page 移动并显示上一页的应用按钮
     */
    void move_to_previous_page();

    /**
     * @brief has_previous_page     是否存在上一页的应用按钮
     * @return 存在返回true，不存在返回false
     */
    bool has_previous_page();

    /**
     * @brief has_next_page         是否存在下一页的应用按钮
     * @return 存在返回true，不存在返回false
     */
    bool has_next_page();

    /**
     * @brief signal_page_changed   信号：当前任务栏显示的应用按钮页面发生变化时触发
     * @return
     */
    sigc::signal<void> signal_page_changed();

protected:
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual void on_add(Gtk::Widget *child) override;
    virtual void on_remove(Gtk::Widget *child) override;
    virtual void on_map() override;
    virtual void on_realize() override;
    virtual void on_unrealize() override;

    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection_data, guint info, guint time) override;

    /**
     * @brief 回调函数，当应用按钮child被拖动时调用
     * @param child 被拖动的应用按钮
     * @param x     拖动后按钮child的x坐标(基于当前控件)
     * @param y     拖动后按钮child的y坐标(基于当前控件)
     */
    virtual void on_button_drag_motion(Gtk::Widget *child, int x, int y);

    /**
     * @brief 回调函数，当应用按钮child拖放结束时调用
     * @param child 被拖动的应用按钮
     */
    virtual void on_button_drag_end(Gtk::Widget *child);

    /**
     * @brief on_orientation_changed 应用按钮排列方向发生变化时的回调函数
     */
    virtual void on_orientation_changed();

    /**
     * @brief 初始化分页状态变化的监视器
     */
    virtual void init_paging_monitor();

    /**
     * @brief put_child_before 将子控件source放到子控件dest之前
     * @param source           待移动的控件
     * @param dest             目标控件
     */
    virtual void put_child_before(Gtk::Widget *source, Gtk::Widget *dest);

    /**
     * @brief put_child_after  将子控件source放到子控件dest之后
     * @param source           待移动的控件
     * @param dest             目标控件
     */
    virtual void put_child_after(Gtk::Widget *source, Gtk::Widget *dest);

    /**
     * @brief child_is_after   检查子控件w1位置是否位于子控件w2之后
     * @param w1               子控件
     * @param w2               子控件
     * @return  w1位于w2之后返回true，否则返回false
     */
    virtual bool child_is_after(Gtk::Widget *w1, Gtk::Widget *w2);
    /**
     * @brief child_is_before  检查子控件w1位置是否位于子控件w2之前
     * @param w1               子控件
     * @param w2               子控件
     * @return w1位于w2之前返回true，否则返回false
     */
    virtual bool child_is_before(Gtk::Widget *w1, Gtk::Widget *w2);

    /**
     * @brief 加载系统已打开应用列表
     */
    virtual void load_applications();

    /**
     * @brief on_active_window_changed 回调函数：当前活动和窗口变化时调用
     * @param previous  上一个活动窗口
     * @param active    当前活动窗口
     */
    void on_active_window_changed(KiranWindowPointer previous, KiranWindowPointer active);

    /**
     * @brief 回调函数：当前活动工作区发生变化时调用
     * @param last_active   上一个活动工作区
     * @param active        当前活动工作区
     * @return (void)
     */
    void on_active_workspace_changed(std::shared_ptr<Kiran::Workspace> last_active,
                                     std::shared_ptr<Kiran::Workspace> active);

    /**
     * @brief on_window_opened  回调函数：新窗口打开时调用
     * @param window    新窗口
     */
    void on_window_opened(KiranWindowPointer window);

    /**
     * @brief on_window_closed  回调函数：窗口关闭时调用
     * @param window    关闭的窗口
     */
    void on_window_closed(KiranWindowPointer window);

    /**
     * @brief 回调函数: 将应用固定到任务栏时调用
     * @param apps   新固定到任务栏的应用列表
     */
    virtual void on_fixed_apps_added(const Kiran::AppVec &apps);

    /**
     * @brief 回调函数: 将应用取消固定到任务栏时调用
     * @param apps   不再固定到任务栏的应用列表
     */
    virtual void on_fixed_apps_removed(const Kiran::AppVec &apps);

    /**
     * @brief 回调函数: 当任务栏应用按钮显示策略发生变化时调用
     *        @see Kiran::TaskbarSkeleton::AppShowPolicy
     * @return
     */
    virtual void on_app_show_policy_changed();

    /**
     * @brief 确保指定的应用按钮在任务栏上可见。任务栏上空间不足时，通过滚动来保证可见
     * @param button    需要可见的应用按钮
     */
    virtual void ensure_active_app_button_visible();

    /**
     * @brief 切换到应用按钮所在的页面，该接口只有在任务栏空间不足时应用按钮分页后才有意义
     * @param button 需要切换显示的按钮
     */
    virtual void switch_to_page_of_button(TasklistAppButton *button);

    /**
     * @brief 初始化界面元素
     */
    virtual void init_ui();

    /**
     * @brief 检查鼠标位置，以决定是否显示预览窗口以及预览窗口位置
     */
    void check_and_toggle_previewer();

    /**
     * @brief 设置定时器，延时检查鼠标位置
     */
    void schedule_pointer_check();

    /**
     * @brief 清除检查鼠标位置的定时器
     */
    void stop_pointer_check();

    /**
     * @brief   find_app_button 查找给定的app对应的应用按钮
     * @param   app 需查找的应用
     * @return  返回app对应的应用按钮，如果未找到，返回nullptr
     */
    TasklistAppButton *find_app_button(const KiranAppPointer &app);

    /**
     * @brief   add_app_button 将给定的app创建应用按钮并添加到任务栏，如果该app对应的按钮已经存在，什么都不做。
     * @param   app 需要添加的应用
     */
    void add_app_button(const KiranAppPointer &app);

    /**
     * @brief   remove_app_button 将给定的app对应的应用按钮从任务栏上移除，如果该app对应的按钮不存在，什么也不做
     * @param   app 需要移除的应用
     */
    void remove_app_button(const KiranAppPointer &app);

    /**
     * @brief move_previewer 移动预览窗口到指定的应用按钮，并显示该应用的窗口预览信息
     * @param target         要显示预览窗口的应用按钮
     */
    void move_previewer(TasklistAppButton *target);

    /**
     * @brief hide_previewer 隐藏预览窗口
     */
    void hide_previewer();

    /**
     * @brief get_previewer_position 根据任务栏面板的排列方向计算预览窗口的显示位置
     * @return 返回预览窗口的显示位置
     */
    Gtk::PositionType get_previewer_position();

    /**
     * @brief get_adjustment 获取滚动区域的Adjustment控件，该控件对应的滚动方向由面板方向决定
     * @return 返回滚动区域的Adjustment控件
     */
    Glib::RefPtr<Gtk::Adjustment> get_adjustment();

    /**
     * @brief get_child_geometry 获取给定控件的大小和位置信息，位置信息是基于root窗口坐标
     * @param child         待获取信息的控件
     * @param rect          用来存放控件信息的结构体
     */
    void get_child_geometry(Gtk::Widget *child, Gdk::Rectangle &rect);

    /**
     * @brief get_pointer_position  获取当前鼠标的屏幕坐标
     * @param pointer_x     鼠标X坐标
     * @param pointer_y     鼠标Y坐标
     */
    void get_pointer_position(int &pointer_x, int &pointer_y);

    /**
     * @brief reorder_child 按钮拖动时，根据鼠标移动方向和鼠标当前位置，重新排列放置子控件widget
     * @param widget        被拖动的应用按钮
     * @param motion_dir    按钮拖动方向
     */
    void reorder_child(Gtk::Widget *widget, PointerMotionDirection motion_dir);

private:
    void init_dnd();

private:
    MatePanelApplet *applet;                                     //所属的面板插件
    std::map<KiranAppPointer, TasklistAppButton *> app_buttons;  //任务栏应用按钮列表

    KiranAppPointer active_app;       //当前活动窗口所属app(cached，不一定是最新的)
    TasklistAppPreviewer *previewer;  //应用预览窗口

    sigc::signal<void> m_signal_page_changed;
    int child_spacing;  //应用按钮间隔
    int n_child_page;   //可视区域内的应用按钮个数

    Glib::Property<Gtk::Orientation> m_property_orient;  //应用按钮排列方向

    sigc::connection pointer_check;       //预览窗口显示状态切换检查定时器
    sigc::connection paging_notify;       //应用按钮页面发生变化时的回调函数
    sigc::connection adjustment_changed;  //分页数据发生变化时的回调函数

    Gtk::Widget *dragging_source;
    Glib::RefPtr<Gdk::Pixbuf> dragging_icon;
    Gdk::Point dragging_pos;
};

#endif  // TASKLIST_BUTTONS_CONTAINER_H
