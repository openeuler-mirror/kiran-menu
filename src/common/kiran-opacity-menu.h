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

#ifndef KIRANOPACITYMENU_H
#define KIRANOPACITYMENU_H

#include <gtkmm.h>

class KiranOpacityMenu : public Gtk::Menu
{
public:
    KiranOpacityMenu();
    virtual ~KiranOpacityMenu() = default;

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
};

#endif  // KIRANOPACITYMENU_H
