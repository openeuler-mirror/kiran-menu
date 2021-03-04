#include "tasklist-app-context-menu.h"
#include "global.h"
#include <menu-skeleton.h>
#include <glib/gi18n.h>
#include "kiran-helper.h"
#include "log.h"

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

    if (!app_) {
        LOG_WARNING("KiranAppContextMenu: found finalized app\n");
        return;
    }

    for (auto action_name: app_->get_actions()) {
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
    } else {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Close all windows"));
        item->signal_activate().connect(
            [this]() -> void {
                auto app_ = app.lock();
                if (app_)
                    app_->close_all_windows();
            });
    }
    append(*item);


    if (!KiranHelper::app_is_in_favorite(app_)) {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Add to favorites"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::add_app_to_favorite), app.lock())));
    } else {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from favorites"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::remove_app_from_favorite), app.lock())));
    }

    if (app_->get_kind() == Kiran::AppKind::FAKE_DESKTOP) {
        /*
         * 无desktop文件的app无法添加到收藏夹
         */
        item->set_sensitive(false);
    }
    append(*item);

    if (!KiranHelper::app_is_in_fixed_list(app_)) {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Pin to taskbar"));
        item->signal_activate().connect(
            sigc::hide_return(
                sigc::bind(sigc::ptr_fun(KiranHelper::add_app_to_fixed_list), app.lock())));
    } else {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Unpin to taskbar"));
        item->signal_activate().connect(
            [this]() -> void {
                auto app_ = app.lock();
                if (app_) {
                    KiranHelper::remove_app_from_fixed_list(app_);
                    if (app_->get_kind() == Kiran::AppKind::USER_TASKBAR) {
                        /* 从磁盘上删除该应用 */
                        Kiran::AppManager::get_instance()->remove_app_from_disk(app_->get_desktop_id());
                    }
                }
            });
    }

    if (app_->get_kind() == Kiran::AppKind::FAKE_DESKTOP) {
        /*
         * 无desktop文件的app无法固定到任务栏
         */
        item->set_sensitive(false);
    }
    append(*item);
}
