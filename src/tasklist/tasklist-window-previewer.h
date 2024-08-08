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
     * @brief on_window_state_changed 回调函数，窗口状态发生变化时调用
     */
    void on_window_state_changed();

    /*混成器变化响应*/
    void on_composite_changed();
    /*gsettings 配置变化响应*/
    void on_settings_changed(const Glib::ustring &changed_key);

private:
    /*显示布局调整*/
    void refresh_layout();
    void refresh_layout(bool is_simply);

private:
    TasklistWindowContextMenu *context_menu;                /* 右键菜单 */
    sigc::signal<void, bool> m_signal_context_menu_toggled; /* 右键菜单打开或关闭的信号 */

    sigc::connection window_state_change; /* 窗口状态变化监控 */
    bool needs_attention;                 /* 窗口是否需要注意 */
    Gdk::RGBA attention_color;            /* 窗口需要注意时的提示颜色 */

    Glib::RefPtr<Gio::Settings> settings; /* 任务栏gsettings*/
};

#endif  // TASKLIST_WINDOW_PREVIEWER_H
