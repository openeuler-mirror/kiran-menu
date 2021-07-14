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

#ifndef MENU_AVATAR_WIDGET_H
#define MENU_AVATAR_WIDGET_H

#include <gtkmm.h>
#include "menu-user-info.h"

// 开始菜单当前用户头像显示控件
class MenuAvatarWidget : public Gtk::Button
{
public:
    /**
     * @brief 构造函数
     * @param size 用户头像显示尺寸
     */
    MenuAvatarWidget(int size);

    /**
     * @brief 设置用户头像显示大小
     * @param size_ 用户头像显示尺寸
     */
    void set_icon_size(int size_);

protected:
    virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr) override;
    virtual void on_clicked() override;

    /**
     * @brief 空闲时加载用户头像
     * @return (void)
     */
    void load_user_info();

    /**
     * @brief 加载用户信息，无延时
     */
    void load_user_info_();

private:
    int icon_size;          /* 用户头像显示大小 */
    MenuUserInfo user_info; /* 当前用户信息 */
};

#endif  // MENU_AVATAR_WIDGET_H
