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

#ifndef TASKLIST_WINDOW_PREVIEWER_H
#define TASKLIST_WINDOW_PREVIEWER_H

#include <gtkmm.h>
//
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
#include <sigc++/sigc++.h>
#include "tasklist-window-context-menu.h"
#include "window-thumbnail-widget.h"
#include "window.h"

class TasklistWindowPreviewer : public WindowThumbnailWidget
{
public:
    TasklistWindowPreviewer(std::shared_ptr<Kiran::Window> &window);
    virtual ~TasklistWindowPreviewer() override;

    /**
     * @brief context_menu_is_opened  右键菜单是否已经打开
     * @return 右键菜单打开返回true,否则返回false
     */
    bool context_menu_is_opened();

    /**
     * @brief signal_context_menu_toggled  信号: 右键菜单打开或关闭时触发，bool参数表示右键菜单是否打开
     * @return 信号
     */
    sigc::signal<void, bool> signal_context_menu_toggled();

protected:
    virtual void get_preferred_width_vfunc(int &minimum_width,
                                           int &natural_width) const override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_button_press_event(GdkEventButton *event) override;
    virtual bool draw_thumbnail_image(Gtk::Widget *snapshot_area, const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual void on_thumbnail_clicked() override;

    /**
     * @brief on_composite_changed  回调函数，窗口管理器复合状态(composite)打开或关闭时调用
     */
    void on_composite_changed();

    /**
     * @brief on_window_state_changed 回调函数，窗口状态发生变化时调用
     */
    void on_window_state_changed();

private:
    TasklistWindowContextMenu *context_menu;                /* 右键菜单 */
    sigc::signal<void, bool> m_signal_context_menu_toggled; /* 右键菜单打开或关闭的信号 */

    sigc::connection window_state_change; /* 窗口状态变化监控 */
    bool needs_attention;                 /* 窗口是否需要注意 */
    Gdk::RGBA attention_color;            /* 窗口需要注意时的提示颜色 */
};

#endif  // TASKLIST_WINDOW_PREVIEWER_H
