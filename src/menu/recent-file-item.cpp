/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-menu is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "recent-file-item.h"
#include <glibmm/i18n.h>
#include "global.h"
#include "kiran-opacity-menu.h"
#include "lib/base.h"

RecentFileItem::RecentFileItem(const Glib::RefPtr<Gtk::RecentInfo> &info)
    : Gtk::ListBoxRow(),
      info_(info)
{
    init();
}

RecentFileItem::~RecentFileItem()
{
    menu_->popdown();
    menu_.clear();
}

void RecentFileItem::init()
{
    auto widget = Gtk::make_managed<Gtk::EventBox>();
    widget->set_tooltip_text(display_name());
    widget->get_style_context()->add_class("row-box");

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);

    auto image = Gtk::make_managed<Gtk::Image>(info_->get_icon(24));
    box->add(*image);

    auto label = Gtk::make_managed<Gtk::Label>(info_->get_display_name());
    label->set_ellipsize(Pango::ELLIPSIZE_END);
    label->set_xalign(0.0f);
    box->add(*label);

    widget->add(*box);
    add(*widget);

    init_context_menu(widget);
}

void RecentFileItem::init_context_menu(Gtk::Widget *widget)
{
    widget->add_events(Gdk::KEY_PRESS_MASK);

    menu_ = Glib::RefPtr<KiranOpacityMenu>(new KiranOpacityMenu());

    auto item = Gtk::make_managed<Gtk::MenuItem>(_("Open File"));
    item->signal_activate().connect(sigc::mem_fun(*this, &RecentFileItem::open_file));
    menu_->append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Open File Location"));
    item->signal_activate().connect(sigc::mem_fun(*this, &RecentFileItem::open_file_lcation));
    menu_->append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from list"));
    item->signal_activate().connect(sigc::mem_fun(*this, &RecentFileItem::remove_file_from_list));
    menu_->append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Clear the whole list"));
    item->signal_activate().connect(sigc::mem_fun(*this, &RecentFileItem::clear_files_list));
    menu_->append(*item);

    menu_->show_all();

    widget->signal_button_press_event().connect(sigc::mem_fun(*this, &RecentFileItem::item_pressed));
    menu_->attach_to_widget(*widget);
}

bool RecentFileItem::check_exists()
{
    g_return_val_if_fail(!uri().empty(), false);

    const Glib::ustring uri = this->uri();
    auto file = Gio::File::create_for_uri(uri);
    if (!file->query_exists())
    {
        KLOG_WARNING("recent file '%s' does not exist", uri.c_str());
        return false;
    }

    return true;
}

bool RecentFileItem::item_pressed(const GdkEventButton *button_event)
{
    const GdkEvent *event = (const GdkEvent *)button_event;

    if (gdk_event_triggers_context_menu(event))
    {
        /* 右键点击时，选择点击项并显示右键菜单 */
        auto list_box = (Gtk::ListBox *)this->get_parent();
        list_box->select_row(*this);
        this->menu_->popup_at_pointer(event);
        return true;
    }

    return false;
}

void RecentFileItem::open_file()
{
    if (!check_exists())
    {
        Gtk::MessageDialog dialog(_("File does not exist"), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        dialog.set_title(_("System Warning"));
        dialog.run();
        return;
    }

    /* 使用系统默认方式打开文件 */
    Gio::AppInfo::launch_default_for_uri_async(uri());
}

void RecentFileItem::open_file_lcation()
{
    if (!check_exists())
    {
        Gtk::MessageDialog dialog(_("File does not exist"), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
        dialog.set_title(_("System Warning"));
        dialog.run();
        return;
    }

    auto file = Gio::File::create_for_uri(uri());
    auto dir = file->get_parent();

    if (!dir)
    {
        KLOG_WARNING("%s: no parent found for file '%s'", __func__, uri().c_str());
        return;
    }

    Gio::AppInfo::launch_default_for_uri_async(dir->get_uri());
}

void RecentFileItem::remove_file_from_list()
{
    g_return_if_fail(!uri().empty());

    auto manager = Gtk::RecentManager::get_default();
    manager->remove_item(uri());
}

void RecentFileItem::clear_files_list()
{
    Gtk::MessageDialog dialog(_("All recent files information will be lost.\nAre you sure to clear the whole lists?"),
                              true,
                              Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_YES_NO,
                              true);
    dialog.set_title(_("System Warning"));

    if (dialog.run() != Gtk::RESPONSE_YES)
        return;

    auto manager = Gtk::RecentManager::get_default();
    manager->purge_items();
}

Glib::ustring RecentFileItem::uri() const
{
    return info_ ? info_->get_uri() : Glib::ustring();
}

Glib::ustring RecentFileItem::display_name() const
{
    return info_ ? info_->get_display_name() : Glib::ustring();
}
