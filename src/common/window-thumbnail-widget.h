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
    explicit WindowThumbnailWidget(KiranWindowPointer &window_);

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
    virtual bool draw_icon_image(Gtk::Widget *icon_area, const Cairo::RefPtr<Cairo::Context> &cr) override;

    /**
     * @brief 回调函数，绘制窗口缩略图时调用
     * @param snapshot_area       待绘制的缩略图区域，实际为DrawingArea
     * @param cr                  绘制相关的Cairo上下文
     * @return                    继续后续的绘制返回false,否则返回true
     */
    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) override;

    /**
     * @brief 根据窗口标题，更新标题标签内容
     */
    virtual void update_title();

private:
    std::weak_ptr<Kiran::Window> window; /* 关联的窗口 */
};

#endif
