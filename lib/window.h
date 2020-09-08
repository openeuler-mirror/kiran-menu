/*
 * @Author       : tangjie02
 * @Date         : 2020-06-08 16:26:46
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 13:58:20
 * @Description  : 该类是对WnckWindow的封装，大部分接口和wnck_window_xxxx相同。
 * @FilePath     : /kiran-menu-2.0/lib/window.h
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
using WinwowGeometry = std::tuple<int, int, int, int>;

class Window : public std::enable_shared_from_this<Window>
{
public:
    Window() = delete;
    Window(const Window& window) = delete;
    Window(WnckWindow* window);
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

    // 将窗口移动到指定工作区
    void move_to_workspace(std::shared_ptr<Workspace> workspace);

    // 获取窗口的window_group，即WnckApplication的xid，同一个进程的窗口应该有相同的window_group
    uint64_t get_window_group();

    // 关闭窗口
    void close();

    // 获取窗口位置和大小
    WinwowGeometry get_geometry();

    // 获取当前所在的工作区。如果窗口为pin状态或者不在任何工作区，则返回空
    std::shared_ptr<Workspace> get_workspace();
    void set_on_visible_workspace(bool on);
    bool get_on_visible_workspace();

    // 窗口标题发生变化信号
    sigc::signal<void> signal_name_changed() { return this->name_changed_; }
    // 工作区变化信号。两个参数分别代表之前的工作区和当前的工作区，如果指针为nullptr，说明窗口不属于任意工作区或者窗口是pin状态
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> signal_workspace_changed() { return this->workspace_changed_; }

private:
    void flush_workspace();

    static void name_changed(WnckWindow* wnck_window, gpointer user_data);
    static void workspace_changed(WnckWindow* wnck_window, gpointer user_data);
    static void geometry_changed(WnckWindow* wnck_window, gpointer user_data);

    void process_events(GdkXEvent* xevent, GdkEvent* event);
    bool update_window_pixmap();

private:
    WnckWindow* wnck_window_;
    GdkWindow* gdk_window_;

    int32_t last_workspace_number_;

    bool last_is_pinned_;

    Pixmap pixmap_;

    sigc::connection load_pixmap_;

    WinwowGeometry last_geometry_;

    sigc::signal<void> name_changed_;
    sigc::signal<void, std::shared_ptr<Workspace>, std::shared_ptr<Workspace>> workspace_changed_;

    friend class WindowManager;
};
}  // namespace Kiran