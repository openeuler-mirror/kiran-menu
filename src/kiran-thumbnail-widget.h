/**
 * @file kiran-thumbnail-widget.h
 * @brief 带标题栏和缩略图区域的缩略图控件
 *        |---------------------------------|
 *        | icon | title              |  X  |
 *        |---------------------------------|
 *        |                                 |
 *        |       thumbnail image           |
 *        |                                 |
 *        |---------------------------------|
 * 
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */

#ifndef KIRAN_THUMBNAIL_WIDGET_INCLUDE_H
#define KIRAN_THUMBNAIL_WIDGET_INCLUDE_H

#include <gtkmm.h>
#include "kiran-helper.h"

class KiranThumbnailWidget: public Gtk::Button
{
public:
    KiranThumbnailWidget();

    /**
     * @brief set_vspacing  设置标题区域和缩略图区域之间的纵向间隔大小
     * @param[in] spacing   设置的间隔大小
     */
    void set_vspacing(int spacing);

    /**
     * @brief 设置标题栏文字
     * @param title_text  要设置的标题栏文字
     */
    void set_title(const Glib::ustring &title_text);

    /**
     * @brief 设置是否显示关闭按钮
     * @param show 是否显示关闭按钮，true表示显示, false表示不显示
     */
    void set_show_close_button(bool show);

    /**
     * @brief 设置是否绘制图标
     * @param show 是否绘制图标，true表示绘制，false表示不绘制
     */
    void set_show_icon(bool show);

protected:
    virtual bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing* crossing_event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    virtual void on_clicked() override;

    /**
     * @brief 回调函数，绘制关闭按钮时调用
     * @param close_area          关闭按钮控件，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_close_button(Gtk::Widget *close_area, const Cairo::RefPtr<Cairo::Context> &cr);

    /**
     * @brief 回调函数，绘制窗口图标时调用
     * @param icon_area           图标区域控件，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr);

    /**
     * @brief 回调函数，绘制窗口缩略图时调用
     * @param snapshot_area       待绘制的缩略图区域，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) = 0;


    /**
     * @brief 回调函数，关闭按钮点击时调用
     */
    virtual void on_close_button_clicked() = 0;

    /**
     * @brief 回调函数，缩略图点击时调用
     */
    virtual void on_thumbnail_clicked() = 0;


    /**
     * @brief 初始化界面
     */
    virtual void init_ui();

    /**
     * @brief         设置缩略图显示区域大小
     * @param width       设置的宽度
     * @param height      设置的高度
     */
    void set_thumbnail_size(int width, int height);

    /**
     * @brief       获取缩略图显示区域控件
     * @return      返回窗口缩略图显示区域控件
     */
    Gtk::Widget *get_thumbnail_area();

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Grid *layout;                  /* 整体布局 */
    Gtk::Label *title_label;            /* 标题标签 */
    Gtk::DrawingArea *icon_area;        /* 图标绘制区域 */
    Gtk::DrawingArea *thumbnail_area;   /* 缩略图绘制区域 */
    Gtk::DrawingArea *close_area;       /* 关闭按钮绘制区域 */


    bool show_close_button;             /* 是否绘制关闭按钮 */
    bool show_icon;                     /* 是否绘制图标 */
};


#endif
