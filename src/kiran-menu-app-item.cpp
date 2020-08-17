#include "kiran-menu-app-item.h"
#include "kiranhelper.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <sys/stat.h>
#include "global.h"

#define MENU_ITEM_COUNT G_N_ELEMENTS(item_labels)

KiranMenuAppItem::KiranMenuAppItem(const std::shared_ptr<Kiran::App> &app_, int _icon_size, Gtk::Orientation orient):
    Glib::ObjectBase("KiranMenuAppItem"),
    KiranMenuListItem(_icon_size, orient),
    app(app_)
{
    auto context = get_style_context();

    context->add_class("kiran-app-item");

    set_text(app_->get_locale_name());

    if (app_->get_icon()) {
        set_icon(app_->get_icon());
    } else {
        g_message("app '%s' has no icon\n", app_->get_file_name().data());
        auto icon = Gio::ThemedIcon::create("application-x-executable");
        set_icon(Glib::RefPtr<Gio::Icon>::cast_dynamic(icon));
    }

    set_tooltip_text(app_->get_locale_comment());

    init_drag_and_drop();
}

const std::shared_ptr<Kiran::App> KiranMenuAppItem::get_app() const
{
    return app.lock();
}

void KiranMenuAppItem::init_drag_and_drop()
{
    std::vector<Gtk::TargetEntry> targets;
    Gtk::TargetEntry target("text/uri-list");

    targets.push_back(target);
    drag_source_set(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY);

    this->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::DragContext> &context) -> void {
            /**
             * 设置拖动操作的Icon
             */
            auto app = this->get_app();
            gtk_drag_set_icon_gicon(context->gobj(), app->get_icon()->gobj(), 0, 0);
        });

    this->signal_drag_data_get().connect(
        [this](const Glib::RefPtr<Gdk::DragContext> &context, Gtk::SelectionData &selection, guint info, guint timestamp) -> void {
            /**
             * 将app对应的desktop文件路径传递给目的控件 
             */
            auto app = this->get_app();
            if (!app) {
                g_warning("init_drag_and_drop: app alreay expired\n");
                return;
            }
            Glib::ustring uri = Glib::filename_to_uri(app->get_file_name()) + "\r\n";

            selection.set(8, (const guint8*)uri.data(), uri.length());
        });

    this->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::DragContext> &context) -> void {
            // 让开始菜单窗口重新获取输入焦点
            Gtk::Container *toplevel = this->get_toplevel();
            KiranHelper::grab_input(*toplevel);
        });

    this->signal_drag_failed().connect(
        [this](const Glib::RefPtr<Gdk::DragContext> &context, Gtk::DragResult result) -> bool {
            g_debug("drag failed, result %d\n", (int)result);
            return true;
        });
}

bool KiranMenuAppItem::on_button_press_event(GdkEventButton *button_event)
{
    if (gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        //鼠标右键点击，显示上下文菜单
        create_context_menu();
        context_menu.popup_at_pointer((GdkEvent*)button_event);
	return false;
    }

    return KiranMenuListItem::on_button_press_event(button_event);
}

void KiranMenuAppItem::on_clicked()
{
    /**
     * 启动应用
     */
    launch_app();
}

bool KiranMenuAppItem::on_key_press_event(GdkEventKey *key_event)
{

    if (key_event->keyval == GDK_KEY_Menu)
    {
        Gtk::Allocation allocation;

        allocation = get_allocation();
        if (get_has_window()) {
            //重新创建右键菜单，以确保收藏夹相关的选项能及时更新
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
    return KiranMenuListItem::on_key_press_event(key_event);
}

void KiranMenuAppItem::create_context_menu()
{
    Gtk::MenuItem *item;
    auto backend = Kiran::MenuSkeleton::get_instance();

    KiranHelper::remove_all_for_container(context_menu);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Launch"));
    item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &KiranMenuAppItem::launch_app)));
    context_menu.append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Add to desktop"));
    item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &KiranMenuAppItem::add_app_to_desktop)));
    context_menu.append(*item);

    if (!is_in_favorite()) {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Add to favorites"));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired()) {
                            auto app = this->app.lock();

                            Kiran::MenuSkeleton::get_instance()->add_favorite_app(app->get_desktop_id());
                        }
                    });
    } else {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from favorites"));
        item->signal_activate().connect(
                    [this]() -> void {
                        if (!this->app.expired()) {
                            auto app = this->app.lock();

                            Kiran::MenuSkeleton::get_instance()->del_favorite_app(app->get_desktop_id());
                        }
                    });
    }

    context_menu.append(*item);

    if (!context_menu.get_attach_widget())
        context_menu.attach_to_widget(*this);
    context_menu.show_all();
}

bool KiranMenuAppItem::add_app_to_desktop()
{
    std::string target_dir;
    Gio::FileCopyFlags flags = Gio::FILE_COPY_BACKUP | Gio::FILE_COPY_TARGET_DEFAULT_PERMS;
    std::shared_ptr<Kiran::App> app;

    if (this->app.expired()) {
        g_warning("%s: app already expired\n", __FUNCTION__);
        return false;
    }

    app = this->app.lock();
    target_dir = Glib::get_user_special_dir(Glib::USER_DIRECTORY_DESKTOP);
    try {
        auto src_file = Gio::File::create_for_path(app->get_file_name());
        auto dest_file = Gio::File::create_for_path(target_dir + "/" + src_file->get_basename());

        //如果桌面上已经存在相同的文件，跳过拷贝操作
        if (dest_file->query_exists())
            return true;

        if (!src_file->copy(dest_file, flags))
            std::cerr<<"Failed to copy"<<std::endl;

        //将desktop文件标记为可执行
        chmod(dest_file->get_path().data(), 0755);
        return true;
    } catch (const Glib::Error &e) {
        std::cerr<<"Error occured while trying to copy desktop file: "<<e.what()<<std::endl;
        return false;
    }
}

bool KiranMenuAppItem::is_in_favorite()
{
    auto app = this->app.lock();
    auto backend = Kiran::MenuSkeleton::get_instance();

    if (!app || backend->lookup_favorite_app(app->get_desktop_id()) == nullptr)
        return false;
    return true;
}

sigc::signal<void> KiranMenuAppItem::signal_launched()
{
    return m_signal_launched;
}

void KiranMenuAppItem::set_orientation(Gtk::Orientation orient)
{
    auto context = get_style_context();

    if (orient == Gtk::ORIENTATION_VERTICAL) {
        set_icon_size(56);
        context->remove_class("kiran-app-item");
        context->add_class("kiran-app-entry");
    } else {
        set_icon_size(24);
        context->remove_class("kiran-app-entry");
        context->add_class("kiran-app-item");
    }

    KiranMenuListItem::set_orientation(orient);
}

void KiranMenuAppItem::launch_app()
{
    if (app.expired()) {
        g_warning("%s: app already expired\n", __FUNCTION__);
        return;
    }
    app.lock()->launch();
    signal_launched().emit();
}
