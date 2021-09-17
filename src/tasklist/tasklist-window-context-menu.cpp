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

#include "tasklist-window-context-menu.h"
#include <glib/gi18n.h>
#include <gtk/gtkx.h>
#include "global.h"
#include "kiran-helper.h"
#include "lib/base.h"
#include "workspace-manager.h"

TasklistWindowContextMenu::TasklistWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_) : win(win_)
{
    refresh();
    get_style_context()->add_class("previewer-context-menu");
}

sigc::signal<void> TasklistWindowContextMenu::signal_window_move_required()
{
    return m_signal_window_move_required;
}

void TasklistWindowContextMenu::refresh()
{
    Gtk::MenuItem *item = nullptr;
    Gtk::RadioMenuItem *radio_item;
    Gtk::CheckMenuItem *check_item;
    Gtk::RadioButtonGroup group;

    KiranHelper::remove_all_for_container(*this);
    auto window = win.lock();

    if (!window)
        return;

    /* 最大化 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Maximize"));
    item->signal_activate().connect(
        [this]() -> void {
            auto window = win.lock();
            if (window)
            {
                if (window->is_minimized())
                {
                    //如果窗口已经最小化，需要先恢复窗口大小
                    window->unminimize(gtk_get_current_event_time());
                }
                window->maximize();
            }
        });
    append(*item);

    /* 最小化 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Minimize"));
    item->signal_activate().connect(
        [this]() -> void {
            auto window = win.lock();
            if (window)
                window->minimize();
        });
    append(*item);

    /* 恢复大小 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Restore"));
    item->signal_activate().connect(
        [this]() -> void {
            auto window = win.lock();
            if (window)
            {
                GdkEvent *event = gtk_get_current_event();

                if (window->is_minimized())
                    window->unminimize(gdk_event_get_time(event));

                if (window->is_maximized())
                    window->unmaximize();
            }
        });
    append(*item);

    /* 移动 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Move"));
    item->signal_activate().connect(
        [this]() -> void {
            auto window = win.lock();
            if (window)
            {
                window->keyboard_move();
            }
        });
    append(*item);

    /* 总是置顶 */
    check_item = Gtk::make_managed<Gtk::CheckMenuItem>(_("Always on top"));
    check_item->signal_activate().connect(
        [this, check_item]() -> void {
            auto window = win.lock();
            if (window)
            {
                if (check_item->get_active())
                    window->make_above();
                else
                    window->make_unabove();
            }
        });
    check_item->set_active(window->is_above());
    append(*check_item);

    /* 仅在当前工作区 */
    radio_item = Gtk::make_managed<Gtk::RadioMenuItem>(group, _("Only on this workspace"));
    radio_item->signal_toggled().connect(
        [this, radio_item]() -> void {
            auto window = win.lock();
            if (window && radio_item->get_active())
                window->set_on_visible_workspace(false);
        });
    append(*radio_item);
    radio_item->set_active(!window->get_on_visible_workspace());

    /* 总在可见工作区 */
    radio_item = Gtk::make_managed<Gtk::RadioMenuItem>(group, _("Always on visible workspace"));
    radio_item->signal_toggled().connect(
        [this, radio_item]() -> void {
            auto window = win.lock();
            if (window && radio_item->get_active())
                window->set_on_visible_workspace(true);
        });
    append(*radio_item);
    radio_item->set_active(window->get_on_visible_workspace());

    /* 移动到其它工作区 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Move to another workspace"));
    auto submenu = create_workspace_submenu();
    item->set_submenu(*submenu);
    submenu->show_all();
    append(*item);

    /* 关闭窗口选项 */
    item = Gtk::make_managed<Gtk::MenuItem>(_("Close Window"));
    item->signal_activate().connect(
        [this]() -> void {
            auto window = win.lock();
            if (!window)
                return;

            window->close();
        });
    append(*item);

    show_all();
}

/**
 * 创建工作区列表菜单
 */
Gtk::Menu *TasklistWindowContextMenu::create_workspace_submenu(void)
{
    Gtk::MenuItem *item;
    auto window = win.lock();

    if (!window)
        return nullptr;

    auto menu = Gtk::make_managed<KiranOpacityMenu>();
    for (auto workspace : Kiran::WorkspaceManager::get_instance()->get_workspaces())
    {
        item = Gtk::make_managed<Gtk::MenuItem>(workspace->get_name());
        item->signal_activate().connect(
            sigc::bind<int>(
                sigc::mem_fun(*this, &TasklistWindowContextMenu::move_window_to_workspace),
                workspace->get_number()));

        menu->append(*item);
        if (workspace == window->get_workspace())
        {
            /* 当前工作区不可点击 */
            item->set_sensitive(false);
        }
    }

    menu->get_style_context()->add_class("previewer-context-menu");
    return menu;
}

/**
 * 将窗口移动到对应的工作区
 */
void TasklistWindowContextMenu::move_window_to_workspace(int workspace_no)
{
    auto window = win.lock();
    auto workspace = Kiran::WorkspaceManager::get_instance()->get_workspace(workspace_no);

    if (window && workspace)
        window->move_to_workspace(workspace);
}
