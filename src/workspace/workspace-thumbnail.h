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

#ifndef WORKSPACE_THUMBNAIL_INCLUDE_H
#define WORKSPACE_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include "kiran-helper.h"
#include "kiran-thumbnail-widget.h"
#include "workspace.h"

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-bg.h>

// 工作区缩略图控件
class WorkspaceThumbnail : public KiranThumbnailWidget
{
public:
    /**
     * @brief 构造函数
     * @param workspace_ 需要关联的工作区对象
     */
    WorkspaceThumbnail(KiranWorkspacePointer &workspace_);
    ~WorkspaceThumbnail() override;

    /**
     * @brief 设置该控件为选中状态.
     *        选中状态下，缩略图会绘制边框.
     *
     * @param selected  是否为选中状态
     */
    void set_selected(bool selected);

    /**
     * @brief  获取当前控件的选中状态
     * @return 已选中返回true，否则返回false
     */
    bool is_selected() const;

    /**
     * @brief 获取关联的工作区对象
     * @return  返回关联的工作区对象
     */
    KiranWorkspacePointer get_workspace();

    /**
     * @brief 回调函数: 重新绘制工作区缩略图背景
     *        通常在桌面壁纸变化时调用
     */
    void on_background_changed();

protected:
    virtual bool on_button_press_event(GdkEventButton *event_button) override;
    virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) override;
    virtual bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time) override;
    virtual void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData &selection, guint info, guint time) override;

    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_close_button_clicked() override;
    virtual void on_thumbnail_clicked() override;

    /**
     * @brief 回调函数: 当插件设置发生变化时调用
     * @see Gio::Settings::signal_changed()
     * @param key 发生变化的Gsettings键名
     * @return 无
     */
    virtual void on_settings_changed(const Glib::ustring &key);

    /**
     * @brief 重新加载缩略图背景
     * @return 加载成功返回true，失败返回false
     */
    bool reload_bg_surface();

    /**
     * @brief 初始化拖放支持，允许将窗口缩略图拖到工作区缩略图上进行窗口的移动
     */
    void init_drag_and_drop();

private:
    Glib::RefPtr<Gio::Settings> bg_settings;     /* 桌面背景设置，用于监控桌面壁纸变化 */
    Glib::RefPtr<Gio::Settings> applet_settings; /* 工作区切换插件设置 */
    std::weak_ptr<Kiran::Workspace> workspace;   /* 关联的工作区对象 */

    cairo_surface_t *bg_surface;       /* 桌面背景 */
    double surface_scale;              /* 绘制窗口缩略图时的缩放比例 */
    int surface_width, surface_height; /* 缓存的bg_surface的宽度和高度 */
    int border_width;                  /* 选中时绘制的缩略图图片边框宽度 */

    Gtk::Widget *thumbnail_area; /* 缩略图绘制区域 */

    bool drop_check;   /* 当前是否处于拖放检测状态，用于区分motion和drop操作 */
    bool draw_windows; /* 是否在工作区缩略图中绘制窗口缩略图 */
};

#endif
