#ifndef TASKLIST_APP_BUTTON_H
#define TASKLIST_APP_BUTTON_H

#include <gtkmm.h>
#include <app.h>
#include <mate-panel-applet.h>
#include "kiran-helper.h"
#include "tasklist-app-context-menu.h"

class TasklistAppButton: public Gtk::Button
{
public:
    enum AppButtonState {
        APP_BUTTON_STATE_NORMAL = 0,        //正常显示
        APP_BUTTON_STATE_ATTENTION,         //需要注意
        APP_BUTTON_STATE_FLICKER            //闪烁（需要注意和常态之间反复切换)
    };
public:
    TasklistAppButton(const std::shared_ptr<Kiran::App> &app, int size);
    ~TasklistAppButton() override;

    /**
     * @brief 按钮拖动信号，对应的参数分别为拖动后按钮相对于父窗口的x坐标和y坐标
     */
    sigc::signal<void, int, int> signal_drag_update();

    /**
     * @brief 按钮拖动结束信号
     */
    sigc::signal<void> signal_drag_end();

    /**
     * @brief get_app_icon_pixbuf 获取应用按钮的图标
     * @return 应用对应的图标
     */
    Glib::RefPtr<Gdk::Pixbuf> get_app_icon_pixbuf();

    /**
     * @brief set_size 设置按钮尺寸
     * @param size  要设置的尺寸
     */
    void set_size(int size);

    /**
     * @brief get_app 获取按钮关联的应用
     * @return  返回关联的应用
     */
    const std::shared_ptr<Kiran::App> get_app();

    /**
     * @brief get_context_menu_opened  获取按钮右键菜单是否已经打开
     * @return 右键菜单已经打开返回true，否则返回false
     */
    bool get_context_menu_opened();

    /**
     * @brief update_windows_icon_geometry
     *        更新应用所有窗口的最小化位置
     */
    void update_windows_icon_geometry();

    /**
     * @brief signal_context_menu_toggled 信号，应用按钮右键菜单打开或关闭时触发，bool参数对应右键菜单的打开状态
     * @return 信号
     */
    sigc::signal<void, bool> signal_context_menu_toggled();

protected:
    virtual Gtk::SizeRequestMode get_request_mode_vfunc() const override;
    virtual void get_preferred_width_vfunc(int &minimum_width, int &natural_width)  const  override;
    virtual void get_preferred_height_vfunc(int &minimum_height, int &natural_height)  const override;
    virtual void get_preferred_width_for_height_vfunc(int width,
                                                      int& minimum_height,
                                                      int& natural_height) const override;
    virtual void get_preferred_height_for_width_vfunc(int width,
                                                     int& minimum_height,
                                                     int& natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual bool on_draw(const::Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual void on_clicked() override;

    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual void on_map() override;

    virtual void on_gesture_drag_update(double x, double y);
    virtual void on_gesture_drag_end(double x, double y);

    virtual void on_drag_data_received(const Glib::RefPtr< Gdk::DragContext >& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time) override;

    /**
     * @brief 回调函数: 新窗口打开时调用
     * @param window 新打开的窗口
     */
    virtual void on_window_opened(const std::shared_ptr<Kiran::Window> &window);

    /**
     * @brief get_orientation  获取应用按钮的排列方向
     * @return 返回按钮的排列方向
     */
    virtual Gtk::Orientation get_orientation() const;

    /**
     * @brief 回调函数：按钮所属应用的窗口中任一窗口状态发生变化时调用
     */
    void on_windows_state_changed();

    /**
     * @brief  为应用按钮绘制需要注意的状态提示
     * @param cr on_draw()信号中的Cairo上下文
     */
    void draw_attentions(const Cairo::RefPtr<Cairo::Context> &cr);

private:
    void init_dnd();
    bool needs_attention();

private:
    TasklistAppContextMenu *context_menu;               //右键菜单
    Gtk::StyleProperty<int> indicator_size_property;    //绘制指示器的尺寸


    int applet_size;                                    //所属插件的尺寸
    int icon_size;                                      //绘制应用图标的尺寸
    std::weak_ptr<Kiran::App> app;                      //关联的app对象

    AppButtonState state;                               //按钮显示状态
    bool dragging;                                      //当前是否处于被拖动状态
    bool pressed;                                       //当前是否处于按下的状态
    GdkPoint drag_point;
    sigc::connection    draw_attention_flicker;         //需要用户注意时的闪烁绘制定时器
    sigc::connection    draw_attention_normal;          //需要用户注意时的最终绘制定时器


    Glib::RefPtr<Gtk::GestureDrag> gesture;                     //拖动的事件
    sigc::signal<void, bool> m_signal_context_menu_toggled;
    sigc::signal<void, int, int> m_signal_drag_update;
    sigc::signal<void> m_signal_drag_end;
};

#endif // TASKLIST_APP_BUTTON_H
