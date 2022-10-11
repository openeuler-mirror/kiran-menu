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

#ifndef __MENU_APPS_CONTAINER_H__
#define __MENU_APPS_CONTAINER_H__

#include <app.h>
#include <gtkmm.h>
#include "menu-app-item.h"
#include "menu-category-item.h"
#include "menu-profile.h"

class MenuAppsContainer : public Gtk::Box
{
public:
    enum AppIconMode
    {
        ICON_MODE_SMALL, /* 小图标，应用名称在图标的右侧 */
        ICON_MODE_LARGE  /* 大图标，应用名称在图标的下方 */
    };

public:
    /**
     * @brief 构造函数
     * @param mode_                 图标显示方式，见AppIconMode
     * @param category_name         应用分类名称
     * @param category_clickable    分类是否可点击
     * @param auto_scroll_          应用过多时是否需要自动滚动
     */
    MenuAppsContainer(AppIconMode mode_,
                      const Glib::ustring &category_name,
                      bool category_clickable,
                      bool auto_scroll_ = false);

    /**
     * @brief set_auto_hide  设置应用列表为空时是否自动隐藏
     * @param auto_hide_     是否要自动隐藏
     */
    void set_auto_hide(bool auto_hide_);

    /**
     * @brief set_draw_frame 设置是否要给本容器绘制边框
     * @param draw_frame_    是否要绘制边框
     */
    void set_draw_frame(bool draw_frame_);

    /**
     * @brief set_empty_prompt_text 设置应用列表为空时显示的提示信息
     * 
     * @param text 要显示的提示信息
     */
    void set_empty_prompt_text(const Glib::ustring &text);

    /**
     * @brief get_category_name 获取当前容器的分类名称
     * 
     * @return 返回当前分类名称
     */
    Glib::ustring get_category_name() const;

    /**
     * @brief get_category_clickable 检查分类标签是否可以点击
     * @return 分类标签可以点击返回true,否则返回false
     */
    bool get_category_clickable() const;

    /**
     * @brief load_applications 加载并显示给定的apps应用列表
     * 
     * @param apps 要加载的应用列表
     * @return 加载成功返回true，失败返回false
     */
    virtual bool load_applications(const Kiran::AppVec &apps);

    /**
     * @brief signal_app_launched 信号，容器内的应用按钮启动时触发
     * 
     * @return 返回应用启动信号
     */
    sigc::signal<void> signal_app_launched();

    /**
     * @brief siganl_category_clicked 信号，容器内的应用分类标签点击后触发
     */
    sigc::signal<void, const Glib::ustring &> signal_category_clicked();

protected:
    /**
     * @brief create_app_item 为给定的app创建应用按钮
     * @param app       给定的应用
     * @param orient    应用按钮内图标和文字的排列方向
     * @return 返回创建的应用按钮
     */
    virtual MenuAppItem *create_app_item(std::shared_ptr<Kiran::App> app,
                                         Gtk::Orientation orient);

    /**
     * @brief get_apps_box 获取用于放置应用按钮的容器，主要在类继承时使用
     * 
     * @return 返回用于放置应用按钮的容器的引用
     */
    Gtk::FlowBox &get_apps_box();

private:
    sigc::signal<void> m_signal_app_launched;
    sigc::signal<void, const Glib::ustring &> m_signal_category_clicked;

    MenuCategoryItem category_box;
    Gtk::ScrolledWindow scrolled_area;
    Gtk::FlowBox apps_box;

    AppIconMode icon_mode;           /* 应用图标显示方式 */
    Glib::ustring empty_prompt_text; /* 应用列表为空时的提示文本 */
    bool auto_hide;                  /* app列表为空时是否要自动隐藏 */
    bool auto_scroll;                /* 是否需要自动滚动 */
};

#endif  // __MENU_APPS_CONTAINER_H__