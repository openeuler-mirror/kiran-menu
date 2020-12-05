#ifndef WINDOW_THUMBNAIL_WIDGET_INCLUDE_H
#define WINDOW_THUMBNAIL_WIDGET_INCLUDE_H

#include <gtkmm.h>
#include <window.h>
#include "kiran-helper.h"

class WindowThumbnailWidget: public Gtk::Button
{
public:
    WindowThumbnailWidget(KiranWindowPointer &window_);

    /**
     * @brief get_window_  获取关联的窗口对象
     * @return             返回关联的窗口对象
     */
    KiranWindowPointer get_window_() const;

    /**
     * @brief set_spacing  设置窗口图标区域和缩略图区域之间的纵向间隔大小
     * @param spacing      设置的间隔大小
     */
    void set_spacing(int spacing);

protected:
    virtual bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    virtual void on_clicked() override;

    /**
     * @brief draw_close_button   回调函数，绘制关闭按钮时调用
     * @param close_area          关闭按钮控件，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_close_button(Gtk::Widget *close_area, const Cairo::RefPtr<Cairo::Context> &cr);

    /**
     * @brief draw_icon_image     回调函数，绘制窗口图标时调用
     * @param icon_area           图标区域控件，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr);

    /**
     * @brief draw_snapshot       回调函数，绘制窗口缩略图时调用
     * @param snapshot_area       待绘制的缩略图区域，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_snapshot(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) = 0;


    /**
     * @brief on_close_button_clicked  回调函数，关闭按钮点击时调用
     */
    virtual void on_close_button_clicked() = 0;

    /**
     * @brief on_thumbnail_clicked     回调函数，缩略图点击时调用
     */
    virtual void on_thumbnail_clicked() = 0;


    /**
     * @brief init_ui                  初始化界面
     */
    virtual void init_ui();

    /**
     * @brief update_title             根据窗口标题，更新标题标签内容
     */
    virtual void update_title();

    /**
     * @brief set_snapshot_size        设置缩略图显示区域大小
     * @param width                    设置的宽度
     * @param height                   设置的高度
     */
    void set_snapshot_size(int width, int height);

    /**
     * @brief get_snapshot_area        获取缩略图显示区域控件
     * @return      返回窗口缩略图显示区域控件
     */
    Gtk::Widget *get_snapshot_area();

private:
    std::weak_ptr<Kiran::Window> window;    /* 关联的窗口 */
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Grid *layout;
    Gtk::Label *title_label;            /* 窗口标题标签 */
    Gtk::DrawingArea *icon_area;        /* 窗口图标绘制区域 */
    Gtk::DrawingArea *snapshot_area;    /* 窗口缩略图绘制区域 */
    Gtk::DrawingArea *close_area;       /* 关闭按钮绘制区域 */
};


#endif
