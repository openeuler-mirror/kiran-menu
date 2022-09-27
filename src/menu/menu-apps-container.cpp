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

#include "menu-apps-container.h"
#include "global.h"
#include "kiran-helper.h"

MenuAppsContainer::MenuAppsContainer(MenuAppsContainer::AppIconMode mode_,
                                     const Glib::ustring &category_name,
                                     bool category_clickable,
                                     bool auto_scroll) : Glib::ObjectBase("MenuAppsContainter"),
                                                         Box(Gtk::ORIENTATION_VERTICAL),
                                                         category_box(category_name, category_clickable),
                                                         icon_mode(mode_),
                                                         auto_hide(false)
{
    apps_box.set_selection_mode(Gtk::SELECTION_NONE);
    apps_box.set_activate_on_single_click(false);
    apps_box.set_vexpand(false);
    apps_box.set_valign(Gtk::ALIGN_START);
    apps_box.get_style_context()->add_class("menu-apps-box");

    category_box.signal_clicked().connect_notify(
        [this]() -> void
        {
            signal_category_clicked().emit(category_box.get_category_name());
        });

    pack_start(category_box, Gtk::PACK_SHRINK);

    if (auto_scroll)
    {
        scrolled_area.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        scrolled_area.add(apps_box);
        pack_start(scrolled_area, Gtk::PACK_EXPAND_WIDGET);
    }
    else
        pack_start(apps_box, Gtk::PACK_EXPAND_WIDGET);
}

void MenuAppsContainer::set_auto_hide(bool auto_hide_)
{
    auto_hide = auto_hide_;
}

void MenuAppsContainer::set_draw_frame(bool draw_frame_)
{
    if (draw_frame_)
        get_style_context()->add_class("menu-section-box");
    else
        get_style_context()->remove_class("menu-section-box");
}

void MenuAppsContainer::set_empty_prompt_text(const Glib::ustring &text)
{
    empty_prompt_text = text;
}

bool MenuAppsContainer::load_applications(const Kiran::AppVec &apps)
{
    MenuAppItem *item = nullptr;

    KiranHelper::remove_all_for_container(apps_box);
    if (apps.size() == 0)
    {
        Gtk::Label *label = nullptr;

        if (auto_hide)
        {
            hide();
            return true;
        }

        label = Gtk::make_managed<Gtk::Label>(empty_prompt_text);
        label->get_style_context()->add_class("empty-prompt-text");
        label->set_hexpand(true);
        label->set_vexpand(true);
        label->set_halign(Gtk::ALIGN_CENTER);
        label->set_valign(Gtk::ALIGN_CENTER);

        apps_box.add(*label);
        apps_box.show_all();
        return true;
    }

    for (auto app : apps)
    {
        auto box_child = Gtk::make_managed<Gtk::FlowBoxChild>();

        if (icon_mode == ICON_MODE_LARGE)
        {
            item = create_app_item(app, Gtk::ORIENTATION_VERTICAL);
            item->set_vexpand(false);
            item->set_valign(Gtk::ALIGN_START);
            apps_box.set_min_children_per_line(1);
            apps_box.set_max_children_per_line(4);
        }
        else
        {
            item = create_app_item(app, Gtk::ORIENTATION_HORIZONTAL);
            item->set_hexpand(true);
            item->set_halign(Gtk::ALIGN_FILL);
            apps_box.set_min_children_per_line(1);
            apps_box.set_max_children_per_line(1);
        }
        apps_box.set_homogeneous(true);

        item = Gtk::manage(item);
        box_child->set_can_focus(false);
        box_child->add(*item);

        apps_box.add(*box_child);
        apps_box.show_all();
    }

    return true;
}

Glib::ustring MenuAppsContainer::get_category_name() const
{
    return category_box.get_category_name();
}

bool MenuAppsContainer::get_category_clickable() const
{
    return category_box.get_clickable();
}

sigc::signal<void, const Glib::ustring &> MenuAppsContainer::signal_category_clicked()
{
    return m_signal_category_clicked;
}

MenuAppItem *MenuAppsContainer::create_app_item(std::shared_ptr<Kiran::App> app, Gtk::Orientation orient)
{
    auto item = new MenuAppItem(app);

    item->set_orientation(orient);

    return item;
}

Gtk::FlowBox &MenuAppsContainer::get_apps_box()
{
    return apps_box;
}
