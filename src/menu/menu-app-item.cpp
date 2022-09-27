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

#include "menu-app-item.h"
#include <glibmm/i18n.h>
#include <sys/stat.h>
#include <taskbar-skeleton.h>
#include <iostream>
#include "global.h"
#include "kiran-helper.h"
#include "lib/base.h"

#define MENU_ITEM_COUNT G_N_ELEMENTS(item_labels)

MenuAppItem::MenuAppItem(const std::shared_ptr<Kiran::App> &app_, int _icon_size, Gtk::Orientation orient) : Glib::ObjectBase("KiranMenuAppItem"),
                                                                                                             MenuListItemWidget(_icon_size, orient),
                                                                                                             app(app_)
{
    auto context = get_style_context();

    context->add_class("menu-app-item");

    set_text(app_->get_locale_name());

    if (app_->get_icon())
    {
        set_icon(app_->get_icon());
    }
    else
    {
        KLOG_INFO("app '%s' has no icon\n", app_->get_file_name().data());
        auto icon = Gio::ThemedIcon::create("application-x-executable");
        set_icon(Glib::RefPtr<Gio::Icon>::cast_dynamic(icon));
    }

    set_can_focus(true);
    set_tooltip_text(app_->get_locale_comment());

    init_drag_and_drop();
}

MenuAppItem::~MenuAppItem()
{
    this->idle_drag_connection_.disconnect();
}

const std::shared_ptr<Kiran::App> MenuAppItem::get_app() const
{
    return app.lock();
}

void MenuAppItem::init_drag_and_drop()
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::TargetEntry target("text/uri-list");

    targets.push_back(target);
    drag_source_set(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY);

    signal_drag_failed().connect(sigc::mem_fun(*this, &MenuAppItem::on_drag_failed));
}

bool MenuAppItem::on_button_press_event(GdkEventButton *button_event)
{
    if (gdk_event_triggers_context_menu((GdkEvent *)button_event))
    {
        //鼠标右键点击，显示上下文菜单
        create_context_menu();
        context_menu.popup_at_pointer((GdkEvent *)button_event);
        return false;
    }

    return MenuListItemWidget::on_button_press_event(button_event);
}

void MenuAppItem::on_clicked()
{
    /*
     * 启动应用
     */
    launch_app();
    get_toplevel()->hide();
}

void MenuAppItem::on_drag_begin(const Glib::RefPtr<Gdk::DragContext> &context)
{
    /*
     * 设置拖动操作的Icon
     */
    auto app = get_app();
    gtk_drag_set_icon_gicon(context->gobj(), app->get_icon()->gobj(), 0, 0);
}

void MenuAppItem::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::SelectionData &selection, guint info, guint timestamp)
{
    /*
     * 将app对应的desktop文件路径传递给目的控件
     */
    auto app = get_app();
    if (!app)
    {
        KLOG_WARNING("init_drag_and_drop: app alreay expired\n");
        return;
    }
    Glib::ustring uri = Glib::filename_to_uri(app->get_file_name()) + "\r\n";

    selection.set(8, (const guint8 *)uri.data(), uri.length());
}

void MenuAppItem::on_drag_end(const Glib::RefPtr<Gdk::DragContext> &context)
{
    /* FIXME: 当拖动图标到任务栏时，无法收到drag-end信号，因为任务栏时kiran-panel的子窗口，kiran-panel是通过
       代理的方式将拖拽信息发送给任务栏，但是任务栏拖拽完成后，这个代理并没有把完成的消息转发给源控件（开始菜单），
       因此无法收到drag-end信号，当前函数也不会被调用。这个时候窗口不再被抓取，需要手动点击左下角开始菜单按钮才能隐藏。*/

    // 如果拖拽被取消，拖拽的ungrab操作可能在drag-end信号之后，所以这里的grab操作放入到后面的事件循环处理。
    if (this->idle_drag_connection_.empty())
    {
        this->idle_drag_connection_ = Glib::signal_idle().connect([this]() -> bool
                                                                  {
                                                                      Gtk::Container *toplevel = this->get_toplevel();
                                                                      KiranHelper::grab_input(*toplevel);
                                                                      return false;
                                                                  });
    }
}

bool MenuAppItem::on_drag_failed(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::DragResult result)
{
    KLOG_DEBUG("drag failed, result %d\n", (int)result);
    return true;
}

bool MenuAppItem::on_key_press_event(GdkEventKey *key_event)
{
    if (key_event->keyval == GDK_KEY_Menu)
    {
        Gtk::Allocation allocation;

        allocation = get_allocation();
        if (get_has_window())
        {
            allocation.set_x(0);
            allocation.set_y(0);
        }

        //重新创建右键菜单，以确保收藏夹相关的选项能及时更新
        create_context_menu();
        context_menu.popup_at_rect(get_window(), allocation,
                                   Gdk::GRAVITY_CENTER,
                                   Gdk::GRAVITY_NORTH_WEST,
                                   (GdkEvent *)key_event);

        return false;
    }
    return MenuListItemWidget::on_key_press_event(key_event);
}

void MenuAppItem::create_context_menu()
{
    Gtk::MenuItem *item;

    KiranHelper::remove_all_for_container(context_menu);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Launch"));
    item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &MenuAppItem::launch_app)));
    context_menu.append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Add to desktop"));
    item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &MenuAppItem::add_app_to_desktop)));
    context_menu.append(*item);

    if (!is_in_favorite())
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Add to favorites"));
        item->signal_activate().connect(sigc::mem_fun(*this, &MenuAppItem::on_add_favorite_app));
    }
    else
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from favorites"));
        item->signal_activate().connect(sigc::mem_fun(*this, &MenuAppItem::on_del_favorite_app));
    }
    context_menu.append(*item);

    if (is_fixed_on_taskbar())
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Unpin to taskbar"));
        item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &MenuAppItem::unpin_app_from_taskbar)));
    }
    else
    {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Pin to taskbar"));
        item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &MenuAppItem::pin_app_to_taskbar)));
    }

    context_menu.append(*item);

    if (!context_menu.get_attach_widget())
        context_menu.attach_to_widget(*this);
    context_menu.show_all();
}

bool MenuAppItem::pin_app_to_taskbar()
{
    auto app_ = app.lock();
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    if (!app_)
    {
        KLOG_WARNING("%s: app already expired\n", __FUNCTION__);
        return false;
    }

    return backend->add_fixed_app(app_->get_desktop_id());
}
bool MenuAppItem::unpin_app_from_taskbar()
{
    auto app_ = app.lock();
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    if (!app_)
    {
        KLOG_WARNING("%s: app already expired\n", __FUNCTION__);
        return false;
    }

    return backend->del_fixed_app(app_->get_desktop_id());
}

bool MenuAppItem::add_app_to_desktop()
{
    std::string target_dir;
    Gio::FileCopyFlags flags = Gio::FILE_COPY_BACKUP | Gio::FILE_COPY_TARGET_DEFAULT_PERMS;
    std::shared_ptr<Kiran::App> app_;

    if (app.expired())
    {
        KLOG_WARNING("%s: app already expired\n", __FUNCTION__);
        return false;
    }

    app_ = app.lock();
    target_dir = Glib::get_user_special_dir(Glib::USER_DIRECTORY_DESKTOP);
    try
    {
        auto src_file = Gio::File::create_for_path(app_->get_file_name());
        auto dest_file = Gio::File::create_for_path(target_dir + "/" + src_file->get_basename());

        //如果桌面上已经存在相同的文件，跳过拷贝操作
        if (dest_file->query_exists())
            return true;

        if (!src_file->copy(dest_file, flags))
        {
            KLOG_WARNING("Failed to copy file");
            return false;
        }

        //将desktop文件标记为可执行
        chmod(dest_file->get_path().data(), 0755);
        return true;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Error occured while trying to copy desktop file: %s", e.what().c_str());
        return false;
    }
}

bool MenuAppItem::is_in_favorite()
{
    auto app_ = app.lock();
    auto backend = Kiran::MenuSkeleton::get_instance();

    if (!app_ || backend->lookup_favorite_app(app_->get_desktop_id()) == nullptr)
        return false;
    return true;
}

bool MenuAppItem::is_fixed_on_taskbar()
{
    auto app_ = app.lock();
    auto backend = Kiran::TaskBarSkeleton::get_instance();

    if (!app_ || backend->lookup_fixed_app(app_->get_desktop_id()) == nullptr)
        return false;

    return true;
}

sigc::signal<void> MenuAppItem::signal_launched()
{
    return m_signal_launched;
}

void MenuAppItem::set_orientation(Gtk::Orientation orient)
{
    auto context = get_style_context();

    if (orient == Gtk::ORIENTATION_VERTICAL)
    {
        set_icon_size(56);
        context->remove_class("horizontal");
        context->add_class("vertical");
    }
    else
    {
        set_icon_size(24);
        context->remove_class("vertical");
        context->add_class("horizontal");
    }

    MenuListItemWidget::set_orientation(orient);
}

void MenuAppItem::launch_app()
{
    if (app.expired())
    {
        KLOG_WARNING("%s: app already expired\n", __FUNCTION__);
        return;
    }

    /**
     * 需要先发出信号，然后再启动。
     * 否则在新应用launch()调用时，后台会触发新安装应用列表变化信号,
     * 当前对象会被销毁，导致无法正常发出启动信号
     */
    signal_launched().emit();
    app.lock()->launch();
}

void MenuAppItem::on_add_favorite_app()
{
    if (!this->app.expired())
    {
        Kiran::MenuSkeleton::get_instance()->add_favorite_app(app.lock()->get_desktop_id());
    }
}

void MenuAppItem::on_del_favorite_app()
{
    if (!app.expired())
    {
        Kiran::MenuSkeleton::get_instance()->del_favorite_app(app.lock()->get_desktop_id());
    }
}
