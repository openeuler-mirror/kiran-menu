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

#ifndef KIRANMENUPROFILE_H
#define KIRANMENUPROFILE_H

#include <gtkmm.h>

typedef enum
{
    DISPLAY_MODE_COMPACT = 0,
    DISPLAY_MODE_EXPAND,
    DISPLAY_MODE_INVALID
} MenuDisplayMode;

typedef enum
{
    PAGE_FAVORITES = 0,
    PAGE_ALL_APPS,
    PAGE_INVALID
} MenuDefaultPage;

class MenuProfile : public sigc::trackable
{
public:
    MenuProfile();
    sigc::signal<void, const Glib::ustring &> signal_changed();

    double get_opacity();
    MenuDisplayMode get_display_mode();
    MenuDefaultPage get_default_page();

    void set_opacity(double value);
    void set_default_page(MenuDefaultPage new_page);
    void set_display_mode(MenuDisplayMode new_mode);

    void on_settings_changed(const Glib::ustring &key);

private:
    Glib::RefPtr<Gio::Settings> settings;

    sigc::signal<void, const Glib::ustring &> m_signal_changed;
};

#endif  // KIRANMENUPROFILE_H
