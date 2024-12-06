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

#ifndef TASKLISTAPPLETWIDGET_H
#define TASKLISTAPPLETWIDGET_H

#include <gtkmm.h>
#include "tasklist-buttons-container.h"

class TasklistAppletWidget : public Gtk::Box
{
public:
    explicit TasklistAppletWidget(MatePanelApplet *applet);

    /**
     * @brief on_app_buttons_page_changed 回调函数，应用按钮当前分页发生变化时调用
     */
    void on_app_buttons_page_changed();

    /**
     * @brief on_applet_orient_changed 回调函数，插件所在面板的排列方向发生变化时调用
     */
    void on_applet_orient_changed();

protected:
    /**
     * @brief init_ui  初始化界面
     */
    void init_ui();

    /**
     * @brief update_action_buttons_state 更新分页按钮状态，当应用按钮所在分页发生变化时调用
     */
    void update_paging_buttons_state();

    /**
     * @brief create_paging_button 创建分页按钮
     * @param icon_resource        按钮图标的资源路径
     * @param tooltip_text         按钮的提示文本
     * @return  返回创建的按钮，该按钮已经调用Gtk::managed接口，将随父控件一起销毁
     */
    Gtk::Button *create_paging_button(const std::string &icon_name, const std::string &tooltip_text);

private:
    Gtk::Box button_box;                 //分页按钮所在布局
    Gtk::Button *prev_btn, *next_btn;    //分页按钮
    TasklistButtonsContainer container;  //按钮容器

    MatePanelApplet *applet;        //所属插件
    sigc::connection paging_check;  //分页按钮状态更新
};

#endif  // TASKLISTAPPLETWIDGET_H
