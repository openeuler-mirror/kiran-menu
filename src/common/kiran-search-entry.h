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

#ifndef KIRANSEARCHENTRY_H
#define KIRANSEARCHENTRY_H

#include <gtkmm.h>

class KiranSearchEntry : public Gtk::SearchEntry
{
public:
    KiranSearchEntry();
    friend void on_preedit_text_changed(KiranSearchEntry *entry, char *preedit_text);

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
    bool has_preedit_text;
};

#endif  // KIRANSEARCHENTRY_H
