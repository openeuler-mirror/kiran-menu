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

#ifndef KIRANCATEGORYITEM_H
#define KIRANCATEGORYITEM_H

#include "menu-list-item-widget.h"

class MenuCategoryItem : public MenuListItemWidget
{
public:
    MenuCategoryItem(const std::string &name, bool clickable);
    const std::string &get_category_name() const;
    bool get_clickable() const;

protected:
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual bool on_enter_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;

    void set_category_name(const std::string &name);
    void set_clickable(bool clickable);

private:
    gboolean clickable;
    std::string category_name;

    const static std::string icon_resource;
};

#endif  // KIRANCATEGORYITEM_H
