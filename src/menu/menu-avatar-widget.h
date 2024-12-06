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
