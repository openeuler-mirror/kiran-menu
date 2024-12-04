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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <gdkmm.h>
// xlib.h must be defined after gdkmm header file.
#include <X11/Xlib.h>
#include <libwnck/libwnck.h>

namespace Kiran
{
class App;
class Window;
class Workspace;

using WindowVec = std::vector<std::shared_ptr<Kiran::Window>>;
using WindowGeometry = std::tuple<int, int, int, int>;

// 该类是对WnckWindow的封装，大部分接口和wnck_window_xxxx相同。
class Window : public std::enable_shared_from_this<Window>
{
public:
    Window() = delete;
    Window(const Window& window) = delete;
    Window(WnckWindow* window);
    Window(gulong xid);
    virtual ~Window();

    static std::shared_ptr<Window> create(WnckWindow* wnck_window);

    // 获取窗口的名字
    std::string get_name();

    // 获取窗口的图标名
    std::string get_icon_name();

    // 获取窗口图标
    GdkPixbuf* get_icon();

    // 获取窗口预览图的pixmap
    Pixmap get_pixmap() { return this->pixmap_; };
    cairo_surface_t* get_thumbnail(int& thumbnail_width, int& thumbnail_height);

    // 获取与该窗口关联的App对象
    std::shared_ptr<App> get_app();

    // 获取改窗口的transient window
    std::shared_ptr<Window> get_transient();

    // 获取窗口的X window ID
    uint64_t get_xid();

    // 获取与当前窗口有相同WM_CLASS属性的窗口集合
    WindowVec get_group_windows();

    // 获取窗口WM_CLASS属性 (res_name field)
    std::string get_class_group_name();

    // 获取窗口WM_CLASS属性 (res_class field)
    std::string get_class_instance_name();

    // 获取窗口对应的进程ID
    int32_t get_pid();

    // 获取窗口类型
    WnckWindowType get_window_type();

    // 窗口是否存在所有工作区中
    bool is_pinned();

    // 窗口是否置顶
    bool is_above();

    bool is_skip_pager();
    bool is_skip_taskbar();

    // 激活窗口
    void activate(uint32_t timestamp);

    // 通过键盘和鼠标移动窗口
    void keyboard_move();

    // 使窗口最小化
    void minimize();
    void unminimize(uint32_t timestamp);
    bool is_minimized();

    // 使窗口最大化
    void maximize();
    bool is_maximized();

    // 还原窗口大小
    void unmaximize();

    //
    bool is_shaded();

    // 使窗口在所有工作区可见
    void pin();

    // 取消窗口在所有工作区可见
    void unpin();

    // 窗口置顶
    void make_above();
    void make_unabove();

    // 判断窗口是否为激活状态
    bool is_active();

    // 判断窗口是否需要用户注意
    bool needs_attention();

    // 将窗口移动到指定工作区
    void move_to_workspace(std::shared_ptr<Workspace> workspace);

    // 获取窗口的window_group，即WnckApplication的xid，同一个进程的窗口应该有相同的window_group
    uint64_t get_window_group();

    // 关闭窗口
    void close();

    // 获取窗口位置和大小，get_geometry函数包含窗口管理器添加边框的大小，如果需要获取(未被窗口管理器处理过的)实际大小，可以使用get_client_window_geometry
    WindowGeometry get_geometry();
    WindowGeometry get_client_window_geometry();

    // 设置窗口的位置和大小，位置和大小包含了窗口管理器添加的边框大小
    void set_geometry(WnckWindowGravity gravity,
                      WnckWindowMoveResizeMask geometry_mask,
                      int x,
                      int y,
                      int width,
                      int height);

    // 获取当前所在的工作区。如果窗口为pin状态或者不在任何工作区，则返回空
    std::shared_ptr<Workspace> get_workspace();
    void set_on_visible_workspace(bool on);
    bool get_on_visible_workspace();

    // 是否应当在任务栏上显示
    bool should_skip_taskbar();

    void set_icon_geometry(int x, int y, int width, int height);

    // 窗口标题发生变化信号
    sigc::signal<void> signal_name_changed() { return this->name_changed_; }
    sigc::signal<void> signal_state_changed() { return this->m_signal_state_changed; }
    // 工作区变化信号。两个参数分别代表之前的工作区和当前的工作区，如果指针为nullptr，说明窗口不属于任意工作区或者窗口是pin状态
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> signal_workspace_changed() { return this->workspace_changed_; }

private:
    void flush_workspace();

    static void name_changed(WnckWindow* wnck_window, gpointer user_data);
    static void workspace_changed(WnckWindow* wnck_window, gpointer user_data);
    static void geometry_changed(WnckWindow* wnck_window, gpointer user_data);
    static void state_changed(WnckWindow* wnck_window, gpointer user_data);

    void process_events(GdkXEvent* xevent, GdkEvent* event);
    bool update_window_pixmap();

private:
    WnckWindow* wnck_window_;
    GdkWindow* gdk_window_;
    gulong xid_;

    int32_t last_workspace_number_;

    bool last_is_pinned_;

    Pixmap pixmap_;

    sigc::connection load_pixmap_;

    WindowGeometry last_geometry_;

    uint64_t name_changed_handler_;
    uint64_t workspace_changed_handler_;
    uint64_t geometry_changed_handler_;
    uint64_t state_changed_handler;

    sigc::signal<void> name_changed_;
    sigc::signal<void> m_signal_state_changed;
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> workspace_changed_;

    friend class WindowManager;
};
}  // namespace Kiran
