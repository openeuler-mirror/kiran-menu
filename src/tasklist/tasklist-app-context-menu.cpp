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

#include "tasklist-app-context-menu.h"
#include <glib/gi18n.h>
#include <menu-skeleton.h>
#include "global.h"
#include "kiran-helper.h"
#include "lib/base.h"

TasklistAppContextMenu::TasklistAppContextMenu(const std::shared_ptr<Kiran::App> &app_)
{
    app = app_;
    get_style_context()->add_class("previewer-context-menu");
    refresh();
}

void TasklistAppContextMenu::refresh()
{
    Gtk::MenuItem *item = nullptr;
    std::shared_ptr<Kiran::App> app_ = app.lock();

    //清空之前的菜单项
    KiranHelper::remove_all_for_container(*this);

    if (!app_)
    {
        KLOG_WARNING("KiranAppContextMenu: found finalized app\n");
        return;
    }

    for (auto action_name : app_->get_actions())
    {
        item = Gtk::make_managed<Gtk::MenuItem>(app_->get_action_name(action_name));
        item->signal_activate().connect(
            [action_name, this]() -> void {
                if (!app.expired())
                    app.lock()->launch_action(action_name);
            });
        append(*item);
    }

    /* 已打开窗口应用显示 "关闭所有窗口"选项，无窗口应用显示"启动"选项 */
    if (app_->get_taskbar_windows().size() == 0)
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Launch"));
        item->signal_activate().connect(
            [this]() -> void {
                auto app_ = app.lock();
                if (app_)
                    app_->launch();
            });
    }
    else
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Close all windows"));
        item->signal_activate().connect(
            [this]() -> void {
                auto app_ = app.lock();
                if (app_)
                    app_->close_all_windows();
            });
    }
    append(*item);

    if (!KiranHelper::app_is_in_favorite(app_))
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Add to favorites"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::add_app_to_favorite), app.lock())));
    }
    else
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from favorites"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::remove_app_from_favorite), app.lock())));
    }

    if (app_->get_kind() == Kiran::AppKind::FAKE_DESKTOP)
    {
        /*
         * 无desktop文件的app无法添加到收藏夹
         */
        item->set_sensitive(false);
    }
    append(*item);

    if (!KiranHelper::app_is_in_fixed_list(app_))
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Pin to taskbar"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::add_app_to_fixed_list), app.lock())));
    }
    else
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Unpin to taskbar"));
        item->signal_activate().connect(
            [this]() -> void {
                auto app_ = app.lock();
                if (app_)
                {
                    KiranHelper::remove_app_from_fixed_list(app_);
                    if (app_->get_kind() == Kiran::AppKind::USER_TASKBAR)
                    {
                        /* 从磁盘上删除该应用 */
                        Kiran::AppManager::get_instance()->remove_app_from_disk(app_->get_desktop_id());
                    }
                }
            });
    }

    if (app_->get_kind() == Kiran::AppKind::FAKE_DESKTOP)
    {
        /*
         * 无desktop文件的app无法固定到任务栏
         */
        item->set_sensitive(false);
    }
    append(*item);
}
