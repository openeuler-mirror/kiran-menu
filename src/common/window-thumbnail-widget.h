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

#ifndef WINDOW_THUMBNAIL_WIDGET_INCLUDE_H
#define WINDOW_THUMBNAIL_WIDGET_INCLUDE_H

#include <gtkmm.h>
#include <window.h>
#include "kiran-helper.h"
#include "kiran-thumbnail-widget.h"

// 窗口缩略图控件
class WindowThumbnailWidget : public KiranThumbnailWidget
{
public:
    /**
     * @brief 构造函数
     * @param window_ 要关联的窗口对象
     */
    WindowThumbnailWidget(KiranWindowPointer &window_);

    /**
     * @brief 获取关联的窗口对象
     * @return  返回关联的窗口对象
     */
    KiranWindowPointer get_window_() const;

protected:
    virtual void on_close_button_clicked() override;

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
    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr);

    /**
     * @brief 根据窗口标题，更新标题标签内容
     */
    virtual void update_title();

private:
    std::weak_ptr<Kiran::Window> window; /* 关联的窗口 */
};

#endif
