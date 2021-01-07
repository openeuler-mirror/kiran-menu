/**
 * @file menu-avatar-widget.h
 * @brief 开始菜单当前用户头像显示控件
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#ifndef MENU_AVATAR_WIDGET_H
#define MENU_AVATAR_WIDGET_H

#include <gtkmm.h>
#include "menu-user-info.h"

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
    virtual bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr) override;
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
    int icon_size;                                                /* 用户头像显示大小 */
    MenuUserInfo user_info;                                       /* 当前用户信息 */
};

#endif // MENU_AVATAR_WIDGET_H
